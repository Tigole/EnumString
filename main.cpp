#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <algorithm>

std::ofstream log;

struct Enum
{
    std::string m_scopePrefix;
    std::string m_enumName;
    std::vector<std::string> m_fields;
};

struct Scope
{
    Scope() : m_scopeName(), m_declarationEndPos(std::string::npos) {}
    Scope(const std::string& scopeName, std::string::size_type declarationEndPos) : m_scopeName(scopeName), m_declarationEndPos(declarationEndPos) {}

    std::string m_scopeName;
    std::string::size_type m_declarationEndPos;
};

void deleteSpaces(std::string& str)
{
    str.erase(std::remove(str.begin(), str.end(), ' '), str.end());
    str.erase(std::remove(str.begin(), str.end(), '\t'), str.end());
    str.erase(std::remove(str.begin(), str.end(), '\n'), str.end());
    str.erase(std::remove(str.begin(), str.end(), '\r'), str.end());
}

void removeComments(std::string& fileContent)
{
    std::string::size_type openCommentPos = 0;
    std::string::size_type closeCommentPos = 0;

    do
    {
        openCommentPos = fileContent.find("/*", openCommentPos);

        if (openCommentPos != std::string::npos)
        {
            closeCommentPos = fileContent.find("*/", openCommentPos);

            fileContent.erase(openCommentPos, closeCommentPos - openCommentPos + 2);
        }
    }
    while (openCommentPos != std::string::npos);

    openCommentPos = 0;
    do
    {
        openCommentPos = fileContent.find("//", openCommentPos);

        if (openCommentPos != std::string::npos)
        {
            closeCommentPos = fileContent.find("\n", openCommentPos);

            fileContent.erase(openCommentPos, closeCommentPos - openCommentPos);
        }
    }
    while (openCommentPos != std::string::npos);
}

bool isClassForwardDeclaration(const std::string& fileContent, std::string::size_type classKeywordPos)
{
    bool isForwardDeclaration = false;
    std::string::size_type openBracketPos = fileContent.find('{', classKeywordPos);
    std::string::size_type semicolonPos = fileContent.find(';', classKeywordPos);

    isForwardDeclaration = semicolonPos < openBracketPos;

    return isForwardDeclaration;
}

std::string::size_type findScopeEnd(const std::string& fileContent, std::string::size_type scopeKeywordPos)
{
    int openingBracketCount = 1;
    std::string::size_type currentPos = fileContent.find('{', scopeKeywordPos) + 1;
    std::string::size_type openingBracketPos;
    std::string::size_type closingBracketPos;

    while (openingBracketCount > 0)
    {
        openingBracketPos = fileContent.find('{', currentPos);
        closingBracketPos = fileContent.find('}', currentPos);

        if (openingBracketPos < closingBracketPos)
        {
            openingBracketCount++;
            currentPos = openingBracketPos;
        }
        else
        {
            openingBracketCount--;
            currentPos = closingBracketPos;
        }

        currentPos++;
    }

    return currentPos;
}

std::string getScopeScopeName(std::vector<Scope>& scopeStack)
{
    std::string returnedValue = "";

    for (auto it = scopeStack.begin(); it != scopeStack.end(); it++)
    {
        returnedValue += it->m_scopeName + "::";
    }

    return returnedValue;
}

std::string getEnumName(const std::string& fileContent,
    const std::string& scopeName,
    std::string::size_type enumKeywordPos,
    std::string::size_type openBracketPos,
    std::string::size_type closeBracketPos,
    std::string::size_type semicolonPos)
{
    std::string::size_type typedefPos = fileContent.rfind("typedef", enumKeywordPos);
    std::string::size_type previousSemicolonPos = fileContent.rfind(';', enumKeywordPos);
    std::string::size_type classPos = fileContent.find("class", enumKeywordPos);
    std::string enumName;

    if (previousSemicolonPos == std::string::npos)
    {
        previousSemicolonPos = 0;
    }

    if ((typedefPos != std::string::npos) && (previousSemicolonPos < typedefPos))
    {
        enumName = fileContent.substr(closeBracketPos + 1, semicolonPos - closeBracketPos - 1);
    }
    else if ((classPos != std::string::npos) && (classPos < openBracketPos))
    {
        std::string::size_type spacePos = fileContent.find(' ', classPos);
        enumName = fileContent.substr(spacePos + 1, openBracketPos - spacePos - 1);
    }
    else
    {
        std::string::size_type spacePos = fileContent.find(' ', enumKeywordPos);
        enumName = fileContent.substr(spacePos + 1, openBracketPos - spacePos - 1);
    }

    deleteSpaces(enumName);

    return scopeName + enumName;
}

std::vector<std::string> getEnumFields(const std::string& fileContent,
    std::string::size_type openBracketPos,
    std::string::size_type closeBracketPos)
{
    std::vector<std::string> fields;
    std::string::size_type currentPos = openBracketPos + 1;
    std::string::size_type comaPos;
    std::string::size_type equalPos;
    std::string tmpField;

    while(currentPos < closeBracketPos)
    {
        comaPos = fileContent.find(',', currentPos);
        equalPos = fileContent.find('=', currentPos);

        if ((comaPos != std::string::npos) && (comaPos < closeBracketPos))
        {
            if ((equalPos != std::string::npos) && (equalPos < comaPos))
            {
                tmpField = fileContent.substr(currentPos, equalPos - currentPos);
            }
            else
            {
                tmpField = fileContent.substr(currentPos, comaPos - currentPos);
            }

            currentPos = comaPos + 1;
        }
        else
        {
            if ((equalPos != std::string::npos) && (equalPos < closeBracketPos))
            {
                tmpField = fileContent.substr(currentPos, equalPos - currentPos);
            }
            else
            {
                tmpField = fileContent.substr(currentPos, closeBracketPos - currentPos);
            }

            currentPos = closeBracketPos + 1;
        }

        deleteSpaces(tmpField);

        if (tmpField.size() > 0)
        {
            fields.push_back(tmpField);
        }
    }

    return fields;
}

void updateScopeStack(std::vector<Scope>& scopeStack, std::string::size_type pos)
{
	while ((scopeStack.size() > 0) && (scopeStack.back().m_declarationEndPos < pos))
	{
		log << "pos: " << pos << '\n';
		log << "scopeStack.top().first: " << scopeStack.back().m_scopeName << "\n";
		log << "scopeStack.top().second: " << scopeStack.back().m_declarationEndPos << "\n";
		scopeStack.pop_back();
	}

	log << "scopeStack.size: " << scopeStack.size() << '\n';
}

void findEnumsInString(const std::string& fileContent, std::vector<Enum>& enums)
{
    std::string::size_type currentPos;
    std::string::size_type enumKeywordPos;
    std::string::size_type classKeywordPos;
    std::string::size_type structKeywordPos;
    std::string::size_type openBracketPos;
    std::string::size_type closeBracketPos;
    std::string::size_type semicolonPos;
    std::string::size_type namespaceKeywordPos;
    std::vector<Scope> scopeStack;
    Enum tmpEnum;

    for (currentPos = 0; currentPos != std::string::npos;)
    {
        enumKeywordPos = fileContent.find("enum", currentPos);
        classKeywordPos = fileContent.find("class", currentPos);
        structKeywordPos = fileContent.find("struct", currentPos);
        namespaceKeywordPos = fileContent.find("namespace", currentPos);

        if (structKeywordPos < classKeywordPos)
        {
            classKeywordPos = structKeywordPos;
        }

		log << "\n\n";
		log << "currentPos: " << currentPos << '\n';
		log << "enumKeywordPos: " << enumKeywordPos << '\n';
		log << "classKeywordPos: " << classKeywordPos << '\n';

		updateScopeStack(scopeStack, enumKeywordPos);

        if (namespaceKeywordPos < enumKeywordPos)
        {
            const std::string::size_type namespaceNameStartPos = fileContent.find(' ', namespaceKeywordPos);
            const std::string::size_type namespaceNameEndPos = fileContent.find('{', namespaceKeywordPos);
            std::string namespaceName = fileContent.substr(namespaceNameStartPos, namespaceNameEndPos - namespaceNameStartPos);
            const std::string::size_type namespaceEndPos = findScopeEnd(fileContent, namespaceNameEndPos);

            deleteSpaces(namespaceName);

            log << "namespace: '" << namespaceName << "'\n";

            log << "namespaceEndPos: " << namespaceEndPos << '\n';

            log << "updateScopeStack\n";
            updateScopeStack(scopeStack, namespaceEndPos);

            log << "pushing class\n";
            scopeStack.push_back(Scope(namespaceName, namespaceEndPos));
            log << "scopeStack.size: " << scopeStack.size() << '\n';

            currentPos = namespaceNameEndPos;

            log << std::flush;
        }
        else if (classKeywordPos < enumKeywordPos)
        {
            const std::string::size_type classNameStartPos = fileContent.find(' ', classKeywordPos);
            const std::string::size_type colonPos = fileContent.find(':', classKeywordPos);
            openBracketPos = fileContent.find('{', classKeywordPos);
            semicolonPos = fileContent.find(';', classKeywordPos);
            const std::string::size_type classNameEndPos = std::min(openBracketPos, std::min(semicolonPos, colonPos));
            std::string className = fileContent.substr(classNameStartPos, classNameEndPos - classNameStartPos);
            std::string::size_type classEndPos = findScopeEnd(fileContent, classNameEndPos);

            deleteSpaces(className);
            log << "className: '" << className << "'\n";

            if (isClassForwardDeclaration(fileContent, classKeywordPos))
            {
                classEndPos = semicolonPos;
            }
            else
            {
				log << "classEndPos: " << classEndPos << '\n';

				log << "updateScopeStack\n";
				updateScopeStack(scopeStack, classEndPos);

				log << "pushing class\n";
				scopeStack.push_back(Scope(className, classEndPos));
				log << "scopeStack.size: " << scopeStack.size() << '\n';
            }

            currentPos = classNameEndPos;

            log << std::flush;
        }
        else
        {
            if (enumKeywordPos != std::string::npos)
            {
                currentPos = enumKeywordPos + 1;

                openBracketPos = fileContent.find('{', enumKeywordPos);
                closeBracketPos = fileContent.find('}', enumKeywordPos);
                semicolonPos = fileContent.find(';', enumKeywordPos);

                if (	(openBracketPos != std::string::npos)
                    && 	(closeBracketPos != std::string::npos)
                    &&	(semicolonPos != std::string::npos)
                    && 	(openBracketPos < closeBracketPos)
                    &&	(closeBracketPos < semicolonPos))
                {
                    tmpEnum.m_enumName = getEnumName(fileContent, "", enumKeywordPos, openBracketPos, closeBracketPos, semicolonPos);
                    tmpEnum.m_scopePrefix = getScopeScopeName(scopeStack);
                    tmpEnum.m_fields = getEnumFields(fileContent, openBracketPos, closeBracketPos);

					log << "enum name: " << tmpEnum.m_enumName << '\n';

                    enums.push_back(tmpEnum);

                    currentPos = semicolonPos;
                }
            }
            else
            {
                currentPos = std::string::npos;
            }
        }
    }
}

bool findEnums(const std::string& filepath, std::vector<Enum>& enums)
{
    std::ifstream stream;
    std::string fileContent;

    stream.open(filepath);

    if (stream.is_open())
    {
        std::stringstream ss;

        ss << stream.rdbuf();

        fileContent = ss.str();

        removeComments(fileContent);

        findEnumsInString(fileContent, enums);
    }

    return stream.is_open();
}

void writeFunction(std::stringstream& ss, const Enum& e)
{
    ss << "\ninline const char* enumToString(" << e.m_scopePrefix << e.m_enumName << " e, bool shortName = false)\n";
    ss << "{\n";
    ss << "    switch(e)\n";
    ss << "    {\n";
    for (std::size_t ii = 0; ii < e.m_fields.size(); ii++)
    {
        ss << "    case " << e.m_scopePrefix << e.m_enumName << "::" << e.m_fields[ii] << ":\n";
        ss << "        if (shortName) return \"" << e.m_fields[ii] << "\";\n";
        ss << "        return \"" << e.m_scopePrefix << e.m_enumName << "::" << e.m_fields[ii] << "\";\n";
    }
    ss << "    default:\n";
    ss << "        return \"" << e.m_scopePrefix << e.m_enumName << "::???\";\n";
    ss << "    }\n";
    ss << "}\n\n";

    ss << "inline bool enumFromString(const char* str, " << e.m_scopePrefix << e.m_enumName << "& e)\n";
    ss << "{\n";
    for (std::size_t ii = 0; ii < e.m_fields.size(); ii++)
    {
        ss << "    if (strcmp(str, \"" << e.m_scopePrefix << e.m_enumName << "::" << e.m_fields[ii] << "\") == 0)\n";
        ss << "    {\n";
        ss << "        e = " << e.m_scopePrefix << e.m_enumName << "::" << e.m_fields[ii] << ";\n";
        ss << "        return true;\n";
        ss << "    }\n";
    }
    ss << "    return false;\n";
    ss << "}\n";
}

std::string getHeaderName(const std::string& shortSourceFileName)
{
    std::size_t dotPos = shortSourceFileName.rfind('.');
    std::string headerName;

    if (dotPos != std::string::npos)
    {
        headerName = shortSourceFileName.substr(0, dotPos);
    }
    else
    {
        headerName = shortSourceFileName;
    }

    for (std::size_t ii = 0; ii < headerName.size(); ii++)
    {
        headerName[ii] = toupper(headerName[ii]);
    }

    headerName += "_ENUM_TO_STRING_H";

    return headerName;
}

void printEnums(const std::string& sourceFileName, const std::vector<Enum>& enums)
{
    std::stringstream ss;
    std::string shortSourceFileName;
    std::size_t lastSlashPos = sourceFileName.rfind('/');
    std::string headerName;

    if (lastSlashPos != std::string::npos)
    {
        shortSourceFileName = sourceFileName.substr(lastSlashPos + 1);
    }
    else
    {
        shortSourceFileName = sourceFileName;
    }

    headerName = getHeaderName(shortSourceFileName);

    ss << "#ifndef " << headerName << '\n';
    ss << "#define " << headerName << " 1 \n\n";
    ss << "/**\n";
    ss << "    Generated using \"" << APPLICATION_NAME << " - " << GIT_VERSION << "\"\n";
    ss << "**/\n\n";

    ss << "#include \"" << shortSourceFileName << "\"\n";

    ss << "#include <cstring>\n";
    for (std::size_t ii = 0; ii < enums.size(); ii++)
    {
        writeFunction(ss, enums[ii]);
    }

    ss << "\n#endif // " << headerName;

    std::cout << ss.str() << '\n';
}


void uut_findEnumsInString(void)
{
    std::vector<Enum> enums;

    std::string testString = "enum class Toto {A, B};";

    testString += "typedef enum t {C, D} Tata;";

    testString += "enum Tutu {E, F};";

    findEnumsInString(testString, enums);

    log << "enum count: '" << enums.size() << "'\n";

    for (std::size_t ii = 0; ii < enums.size(); ii++)
    {
        log << "\n'" << enums[ii].m_enumName << "'";
        for (std::size_t jj = 0; jj < enums[ii].m_fields.size(); jj++)
        {
            log << "\n\t'" << enums[ii].m_fields[jj] << "'";
        }
    }

    log << "\n\n";
}

void uut_findEnums(void)
{
    std::vector<Enum> enums;

    findEnums("/home/sylvain/Projets/hmi/apps/neosafe_gcsa/dev/src/gcsa_pa/protocolAdaptorDef.h", enums);

    log << "enum count: '" << enums.size() << "'\n";

    for (std::size_t ii = 0; ii < enums.size(); ii++)
    {
        log << "\n'" << enums[ii].m_enumName << "'";
        for (std::size_t jj = 0; jj < enums[ii].m_fields.size(); jj++)
        {
            log << "\n\t'" << enums[ii].m_fields[jj] << "'";
        }
    }

    log << "\n\n";
}

void uut_removeComments()
{
    std::ifstream stream;
    std::string fileContent;

    stream.open("/home/sylvain/Projets/hmi/apps/neosafe_gcsa/dev/src/gcsa_pa/protocolAdaptorDef.h");

    if (stream.is_open())
    {
        std::stringstream ss;

        ss << stream.rdbuf();

        fileContent = ss.str();

        log << "file\n\n" << fileContent << "\n\n";

        removeComments(fileContent);

        log << "file\n\n" << fileContent;
    }

    log << "\n\n";
}



int main(int argc, char** argv)
{
    log.open("/tmp/enumPrinter.log");

    if (argc >= 2)
    {
        const char* file = argv[1];
        std::vector<Enum> enums;

        findEnums(file, enums);

        printEnums(file, enums);
    }
    else
    {
        std::cerr << "argv[1]: header file to get enum definitions\n";
        std::cerr << "To write the output into a file, please use the '>' bash operator\n";
        std::cerr << "Version: " << GIT_VERSION << '\n';
    }

    return 0;
}

/*
  Copyright (c) 2009,2010 iwagaki@users.sourceforge.net

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in
  all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
  THE SOFTWARE.
*/

#ifndef OPTION_PARSER_H_
#define OPTION_PARSER_H_

#include <cstdlib>
#include <vector>
#include <cstring>
#include "verify.h"

class OptionHolder
{
public:
    OptionHolder(const char* pIdentifier, const char* pDescription) :
        m_pIdentifier(pIdentifier), m_pDescription(pDescription) {}

    virtual ~OptionHolder() {}
    virtual void help() = 0;
    virtual void setValue(const char* pString) = 0;

    const char* m_pIdentifier;
    const char* m_pDescription;
};


class OptionHolderInt : public OptionHolder
{
public:
    OptionHolderInt(int value, const char* pIdentifier, const char* pDescription) :
        OptionHolder(pIdentifier, pDescription), m_value(value) {}

    void help()
    {
        printf("    --%-30s  %s (%d)\n", m_pIdentifier, m_pDescription, m_value);
    }

    void setValue(const char* pString)
    {
        VERIFY(pString != 0);
        m_value = atoi(pString);
    }
    
    int m_value;
};


class OptionHolderDouble : public OptionHolder
{
public:
    OptionHolderDouble(double value, const char* pIdentifier, const char* pDescription) :
        OptionHolder(pIdentifier, pDescription), m_value(value) {}

    void help()
    {
        printf("    --%-30s  %s (%f)\n", m_pIdentifier, m_pDescription, m_value);
    }

    void setValue(const char* pString)
    {
        VERIFY(pString != 0);
        m_value = atof(pString);
    }
    
    double m_value;
};


class OptionHolderString : public OptionHolder
{
public:
    OptionHolderString(const char* value, const char* pIdentifier, const char* pDescription) :
        OptionHolder(pIdentifier, pDescription), m_value(value) {}
    
    void help()
    {
        const char* value = m_value ? m_value : "NULL";
        printf("    --%-30s  %s (%s)\n", m_pIdentifier, m_pDescription, value);
    }

    void setValue(const char* pString)
    {
        VERIFY(pString != 0);
        m_value = pString;
    }
    
    const char* m_value;
};


class OptionHolderBool : public OptionHolder
{
public:
    OptionHolderBool(bool value, const char* pIdentifier, const char* pDescription) :
        OptionHolder(pIdentifier, pDescription), m_value(value), m_default(value) {}
    
    void help()
    {
        printf("    --%-30s  %s (%s)\n", m_pIdentifier, m_pDescription, m_value == true ? "true" : "false");
    }

    void setValue(const char* pString)
    {
        VERIFY(pString == 0);
        m_value = !m_default;
    }
    
    bool m_value;
    bool m_default;
};


class OptionParser
{
public:
    OptionParser() {}

    virtual ~OptionParser()
    {
        for (std::vector<OptionHolder*>::iterator i = m_optionList.begin(); i != m_optionList.end(); ++i)
            delete *i;
    }

    static int& createOption(int value, const char* pIdentifier, const char* pDescription)
    {
        OptionHolderInt* p = new OptionHolderInt(value, pIdentifier, pDescription);
        m_optionList.push_back(p);

        return p->m_value;
    }

    static double& createOption(double value, const char* pIdentifier, const char* pDescription)
    {
        OptionHolderDouble* p = new OptionHolderDouble(value, pIdentifier, pDescription);
        m_optionList.push_back(p);

        return p->m_value;
    }

    static const char*& createOption(const char* value, const char* pIdentifier, const char* pDescription)
    {
        OptionHolderString* p = new OptionHolderString(value, pIdentifier, pDescription);
        m_optionList.push_back(p);

        return p->m_value;
    }
    
    static bool& createOption(bool value, const char* pIdentifier, const char* pDescription)
    {
        OptionHolderBool* p = new OptionHolderBool(value, pIdentifier, pDescription);
        m_optionList.push_back(p);

        return p->m_value;
    }

    static void help()
    {
        printf("Options:\n");

        for (std::vector<OptionHolder*>::iterator i = m_optionList.begin(); i != m_optionList.end(); ++i)
            (*i)->help();
    }

    static bool numOfArguments()
    {
        return m_argumentList.size();
    }

    static void parse(int argc, char** argv)
    {
        int index = 1;

        while (index < argc)
        {
            bool isFound = false;

            char* pParameter = 0;
            char* pIdentifier = 0;

            if (!parse1Param(index, pIdentifier, pParameter, argc, argv))
            {
                m_argumentList.push_back(pIdentifier);
                continue;
            }

            for (std::vector<OptionHolder*>::iterator i = m_optionList.begin(); i != m_optionList.end(); ++i)
            {
                if (strcmp(pIdentifier, (*i)->m_pIdentifier) == 0)
                {
                    (*i)->setValue(pParameter);
                    isFound = true;
                    break;
                }
            }

            if (!isFound)
            {
                help();
                exit(-1);
            }
        }
    }

    static bool parse1Param(int& index, char*& pIdentifer, char*& pParameter, int argc, char** argv)
    {
        if (index >= argc)
            VERIFY(0);

        pIdentifer = argv[index++];

        if (strncmp(pIdentifer, "--", 2) != 0)
            return false;
        
        pIdentifer += 2;
        pParameter = 0;

        if (index < argc)
        {
            char* p2 = argv[index];
        
            if (strncmp(p2, "--", 2) != 0)
            {
                pParameter = p2;
                ++index;
            }
        }

        return true;
    }

    static std::vector<char*> m_argumentList;
private:
    static std::vector<OptionHolder*> m_optionList;
};


#endif // OPTION_PARSER_H_

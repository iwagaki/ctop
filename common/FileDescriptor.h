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

#ifndef FILE_DESCRIPTOR_H_
#define FILE_DESCRIPTOR_H_

#include <cstdio>
#include "verify.h"


class FileDescriptor
{
public:
    FileDescriptor(const char* filename, const char* attr = "wb") : m_fp(0)
    {
        VERIFY(filename != 0);

        m_fp = fopen(filename, attr);
//        VERIFY(m_fp != 0);
    }

    virtual ~FileDescriptor()
    {
        if (m_fp)
        {
            if (fclose(m_fp) != 0)
                assert(0);

            m_fp = 0;
        }
    }

    operator FILE*() const
    {
        return fp();
    }

    FILE* fp() const
    {
//        VERIFY(m_fp != 0);
        return m_fp;
    }

private:
    FILE* m_fp;
};


class FileReader : public FileDescriptor
{
public:
    FileReader(const char* filename) : FileDescriptor(filename, "rb") {}
};


class FileWriter : public FileDescriptor
{
public:
    FileWriter(const char* filename) : FileDescriptor(filename, "wb") {}
};


#endif // FILE_DESCRIPTOR_H_

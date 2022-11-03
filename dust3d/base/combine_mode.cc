/*
 *  Copyright (c) 2016-2021 Jeremy HU <jeremy-at-dust3d dot org>. All rights reserved. 
 *
 *  Permission is hereby granted, free of charge, to any person obtaining a copy
 *  of this software and associated documentation files (the "Software"), to deal
 *  in the Software without restriction, including without limitation the rights
 *  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 *  copies of the Software, and to permit persons to whom the Software is
 *  furnished to do so, subject to the following conditions:

 *  The above copyright notice and this permission notice shall be included in all
 *  copies or substantial portions of the Software.

 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 *  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 *  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 *  SOFTWARE.
 */

#include <dust3d/base/combine_mode.h>

namespace dust3d {

CombineMode CombineModeFromString(const char* modeString)
{
    std::string mode = modeString;
    if (mode == "Normal")
        return CombineMode::Normal;
    if (mode == "Inversion")
        return CombineMode::Inversion;
    if (mode == "Uncombined")
        return CombineMode::Uncombined;
    return CombineMode::Normal;
}

const char* CombineModeToString(CombineMode mode)
{
    switch (mode) {
    case CombineMode::Normal:
        return "Normal";
    case CombineMode::Inversion:
        return "Inversion";
    case CombineMode::Uncombined:
        return "Uncombined";
    default:
        return "Normal";
    }
}

std::string CombineModeToDispName(CombineMode mode)
{
    switch (mode) {
    case CombineMode::Normal:
        return std::string("Normal");
    case CombineMode::Inversion:
        return std::string("Inversion");
    case CombineMode::Uncombined:
        return std::string("Uncombined");
    default:
        return std::string("Normal");
    }
}

}

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

#ifndef DUST3D_MESH_HOLE_STITCHER_H_
#define DUST3D_MESH_HOLE_STITCHER_H_

#include <dust3d/base/vector3.h>
#include <dust3d/mesh/hole_wrapper.h>
#include <vector>

namespace dust3d {

class HoleStitcher {
public:
    ~HoleStitcher();
    void setVertices(const std::vector<Vector3>* vertices);
    bool stitch(const std::vector<std::pair<std::vector<size_t>, Vector3>>& edgeLoops);
    const std::vector<std::vector<size_t>>& newlyGeneratedFaces();
    void getFailedEdgeLoops(std::vector<size_t>& failedEdgeLoops);

private:
    const std::vector<Vector3>* m_positions;
    std::vector<std::vector<size_t>> m_newlyGeneratedFaces;
    HoleWrapper* m_wrapper = nullptr;

    bool stitchByQuads(const std::vector<std::pair<std::vector<size_t>, Vector3>>& edgeLoops);
};

}

#endif

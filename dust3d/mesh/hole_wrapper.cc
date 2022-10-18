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

#include <cmath>
#include <dust3d/base/math.h>
#include <dust3d/mesh/hole_wrapper.h>
#include <set>

namespace dust3d {

void HoleWrapper::setVertices(const std::vector<Vector3>* vertices)
{
    m_positions = vertices;
}

void HoleWrapper::wrap(const std::vector<std::pair<std::vector<size_t>, Vector3>>& edgeLoops)
{
    size_t nextPlaneId = 1;
    for (const auto& it : edgeLoops) {
        addCandidateVertices(it.first, it.second, nextPlaneId++);
    }
    generate();
    finalize();
}

const std::vector<std::vector<size_t>>& HoleWrapper::newlyGeneratedFaces()
{
    return m_newlyGeneratedfaces;
}

bool HoleWrapper::finished()
{
    if (!m_finalizeFinished)
        return false;
    if (m_candidates.empty())
        return true;
    for (const auto& it : m_candidates) {
        if (!isVertexClosed(it))
            return false;
    }
    return true;
}

void HoleWrapper::getFailedEdgeLoops(std::vector<size_t>& failedEdgeLoops)
{
    std::set<size_t> edgeLoopIndices;
    for (const auto& it : m_candidates) {
        if (isVertexClosed(it))
            continue;
        edgeLoopIndices.insert(m_sourceVertices[it].sourcePlane - 1);
    }
    for (const auto& index : edgeLoopIndices) {
        failedEdgeLoops.push_back(index);
    }
}

void HoleWrapper::addCandidateVertices(const std::vector<size_t>& vertices, const Vector3& planeNormal, size_t planeId)
{
    std::map<size_t, size_t> verticesIndexSet;
    for (const auto& oldVertId : vertices) {
        if (verticesIndexSet.find(oldVertId) == verticesIndexSet.end()) {
            size_t index = addSourceVertex((*m_positions)[oldVertId], planeId, oldVertId);
            verticesIndexSet.insert({ oldVertId, index });
        }
    }
    for (size_t i = 0; i < vertices.size(); ++i) {
        const auto& oldVertId = vertices[i];
        const auto& oldNextVertId = vertices[(i + 1) % vertices.size()];
        auto vertexIndex = verticesIndexSet[oldVertId];
        auto nextVertexIndex = verticesIndexSet[oldNextVertId];
        addStartup(nextVertexIndex, vertexIndex, planeNormal);
    }
}

size_t HoleWrapper::addSourceVertex(const Vector3& position, size_t sourcePlane, size_t tag)
{
    auto addedIndex = m_sourceVertices.size();

    SourceVertex sourceVertex;
    sourceVertex.position = position;
    sourceVertex.sourcePlane = sourcePlane;
    sourceVertex.tag = tag;
    sourceVertex.index = addedIndex;

    m_sourceVertices.push_back(sourceVertex);
    m_candidates.push_back(addedIndex);

    return addedIndex;
}

void HoleWrapper::addStartup(size_t p1, size_t p2, const Vector3& baseNormal)
{
    if (m_items.empty())
        addItem(p1, p2, baseNormal);
    m_generatedFaceEdgesMap.insert({ WrapItemKey { p2, p1 }, { 0, false } });
}

Vector3 HoleWrapper::calculateFaceVector(size_t p1, size_t p2, const Vector3& baseNormal)
{
    const auto& v1 = m_sourceVertices[p1];
    const auto& v2 = m_sourceVertices[p2];
    auto seg = v2.position - v1.position;
    return Vector3::crossProduct(seg, baseNormal);
}

void HoleWrapper::addItem(size_t p1, size_t p2, const Vector3& baseNormal)
{
    const auto& v1 = m_sourceVertices[p1];
    const auto& v2 = m_sourceVertices[p2];
    if (!m_items.empty() && v1.sourcePlane == v2.sourcePlane)
        return;
    if (findItem(p1, p2).second || findItem(p2, p1).second)
        return;
    if (isEdgeGenerated(p1, p2) || isEdgeGenerated(p2, p1))
        return;
    auto index = m_items.size();
    WrapItem item;
    item.p3 = 0;
    item.p1 = p1;
    item.p2 = p2;
    item.baseNormal = baseNormal;
    item.processed = false;
    m_items.push_back(item);
    m_itemsMap.insert({ WrapItemKey { p1, p2 }, index });
    m_itemsList.push_front(index);
}

std::pair<size_t, bool> HoleWrapper::findItem(size_t p1, size_t p2)
{
    auto key = WrapItemKey { p1, p2 };
    auto findResult = m_itemsMap.find(key);
    if (findResult == m_itemsMap.end()) {
        return { 0, false };
    }
    return { findResult->second, true };
}

bool HoleWrapper::isEdgeGenerated(size_t p1, size_t p2)
{
    auto key = WrapItemKey { p1, p2 };
    if (m_generatedFaceEdgesMap.find(key) == m_generatedFaceEdgesMap.end())
        return false;
    return true;
}

float HoleWrapper::angleOfBaseFaceAndPoint(size_t itemIndex, size_t vertexIndex)
{
    const auto& item = m_items[itemIndex];
    if (item.p1 == vertexIndex || item.p2 == vertexIndex)
        return 0.0;
    const auto& v1 = m_sourceVertices[item.p1];
    const auto& v2 = m_sourceVertices[item.p2];
    const auto& vp = m_sourceVertices[vertexIndex];
    if (v1.sourcePlane == v2.sourcePlane && v1.sourcePlane == vp.sourcePlane)
        return 0.0;
    auto vd1 = calculateFaceVector(item.p1, item.p2, item.baseNormal);
    auto normal = Vector3::normal(v2.position, v1.position, vp.position);
    auto vd2 = calculateFaceVector(item.p1, item.p2, normal);
    return Math::radiansToDegrees(Vector3::angleBetween(vd2, vd1));
}

std::pair<size_t, bool> HoleWrapper::findBestVertexOnTheLeft(size_t itemIndex)
{
    auto p1 = m_items[itemIndex].p1;
    auto p2 = m_items[itemIndex].p2;
    auto maxAngle = 0.0;
    std::pair<size_t, bool> result = { 0, false };
    for (auto it = m_candidates.begin(); it != m_candidates.end(); /*void*/) {
        auto cand = *it;
        if (isVertexClosed(cand)) {
            it = m_candidates.erase(it);
            continue;
        }
        ++it;
        if (isEdgeClosed(p1, cand) || isEdgeClosed(p2, cand))
            continue;
        auto angle = angleOfBaseFaceAndPoint(itemIndex, cand);
        if (angle > maxAngle) {
            maxAngle = angle;
            result = { cand, true };
        }
    }
    return result;
}

std::pair<size_t, bool> HoleWrapper::peekItem()
{
    for (const auto& itemIndex : m_itemsList) {
        if (!m_items[itemIndex].processed) {
            return { itemIndex, true };
        }
    }
    return { 0, false };
}

bool HoleWrapper::isEdgeClosed(size_t p1, size_t p2)
{
    return m_generatedFaceEdgesMap.find(WrapItemKey { p1, p2 }) != m_generatedFaceEdgesMap.end() && m_generatedFaceEdgesMap.find(WrapItemKey { p2, p1 }) != m_generatedFaceEdgesMap.end();
}

bool HoleWrapper::isVertexClosed(size_t vertexIndex)
{
    auto findResult = m_generatedVertexEdgesMap.find(vertexIndex);
    if (findResult == m_generatedVertexEdgesMap.end())
        return false;
    for (const auto& otherIndex : findResult->second) {
        if (!isEdgeClosed(vertexIndex, otherIndex))
            return false;
    }
    return true;
}

void HoleWrapper::generate()
{
    for (;;) {
        auto findItem = peekItem();
        if (!findItem.second)
            break;
        auto itemIndex = findItem.first;
        m_items[itemIndex].processed = true;
        auto p1 = m_items[itemIndex].p1;
        auto p2 = m_items[itemIndex].p2;
        if (isEdgeClosed(p1, p2))
            continue;
        auto findP3 = findBestVertexOnTheLeft(itemIndex);
        if (findP3.second) {
            auto p3 = findP3.first;
            m_items[itemIndex].p3 = p3;
            auto baseNormal = Vector3::normal(m_sourceVertices[p1].position,
                m_sourceVertices[p2].position,
                m_sourceVertices[p3].position);
            auto faceIndex = m_generatedFaces.size();
            m_generatedFaces.push_back(Face3 { p1, p2, p3, baseNormal, faceIndex });
            addItem(p3, p2, baseNormal);
            addItem(p1, p3, baseNormal);
            m_generatedFaceEdgesMap.insert({ WrapItemKey { p1, p2 }, { faceIndex, true } });
            m_generatedFaceEdgesMap.insert({ WrapItemKey { p2, p3 }, { faceIndex, true } });
            m_generatedFaceEdgesMap.insert({ WrapItemKey { p3, p1 }, { faceIndex, true } });
            m_generatedVertexEdgesMap[p1].push_back(p2);
            m_generatedVertexEdgesMap[p1].push_back(p3);
            m_generatedVertexEdgesMap[p2].push_back(p3);
            m_generatedVertexEdgesMap[p2].push_back(p1);
            m_generatedVertexEdgesMap[p3].push_back(p1);
            m_generatedVertexEdgesMap[p3].push_back(p2);
        }
    }
}

size_t HoleWrapper::anotherVertexIndexOfFace3(const Face3& f, size_t p1, size_t p2)
{
    std::vector<size_t> indices = { f.p1, f.p2, f.p3 };
    for (const auto& index : indices) {
        if (index != p1 && index != p2)
            return index;
    }
    return 0;
}

std::pair<size_t, bool> HoleWrapper::findPairFace3(const Face3& f, std::map<size_t, bool>& usedIds, std::vector<Face4>& q)
{
    std::vector<size_t> indices = { f.p1, f.p2, f.p3 };
    for (size_t i = 0; i < indices.size(); ++i) {
        auto j = (i + 1) % indices.size();
        auto k = (i + 2) % indices.size();
        auto findPairedFace3Id = m_generatedFaceEdgesMap.find(WrapItemKey { indices[j], indices[i] });
        if (findPairedFace3Id != m_generatedFaceEdgesMap.end() && findPairedFace3Id->second.second) {
            auto pairedFace3Id = findPairedFace3Id->second.first;
            if (usedIds.find(pairedFace3Id) != usedIds.end())
                continue;
            const auto& pairedFace3 = m_generatedFaces[pairedFace3Id];
            if (!almostEqual(pairedFace3.normal, f.normal))
                continue;
            auto anotherIndex = anotherVertexIndexOfFace3(pairedFace3, indices[j], indices[i]);
            auto mergedF = Face4 { indices[i], anotherIndex, indices[j], indices[k] };
            q.push_back(mergedF);
            return { pairedFace3Id, true };
        }
    }
    return { 0, false };
}

bool HoleWrapper::almostEqual(const Vector3& v1, const Vector3& v2)
{
    return abs(v1.x() - v2.x()) <= 0.01 && abs(v1.y() - v2.y()) <= 0.01 && abs(v1.z() - v2.z()) <= 0.01;
}

void HoleWrapper::finalize()
{
    std::vector<Face4> quads;
    std::map<size_t, bool> usedIds;
    m_finalizeFinished = true;
    for (const auto& f : m_generatedFaces) {
        if (usedIds.find(f.index) != usedIds.end())
            continue;
        usedIds.insert({ f.index, true });
        auto paired = findPairFace3(f, usedIds, quads);
        if (paired.second) {
            usedIds.insert({ paired.first, true });
            continue;
        }
        std::vector<size_t> addedVertices = {
            m_sourceVertices[f.p1].tag,
            m_sourceVertices[f.p2].tag,
            m_sourceVertices[f.p3].tag,
        };
        m_newlyGeneratedfaces.push_back(addedVertices);
    }
    for (const auto& f : quads) {
        std::vector<size_t> addedVertices = {
            m_sourceVertices[f.p1].tag,
            m_sourceVertices[f.p2].tag,
            m_sourceVertices[f.p3].tag,
            m_sourceVertices[f.p4].tag,
        };
        m_newlyGeneratedfaces.push_back(addedVertices);
    }
}

}

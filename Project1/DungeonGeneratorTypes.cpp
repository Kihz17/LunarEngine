#include "DungeonGeneratorTypes.h"
#include "DungeonGenUtils.h"

#include <glm/gtx/norm.hpp>

//////////////////////
// TETRAHEDRON
//////////////////////

Tetrahedron::Tetrahedron(const glm::vec3 & a, const glm::vec3 & b, const glm::vec3 & c, const glm::vec3 & d)
	: a(a), b(b), c(c), d(d)
{
    glm::mat4 aM(1.0f);
    aM[0] = glm::vec4(a.x, b.x, c.x, d.x);
    aM[1] = glm::vec4(a.y, b.y, c.y, d.y);
    aM[2] = glm::vec4(a.z, b.z, c.z, d.z);
    aM[3] = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
    float aD = glm::determinant(aM);

    float aPosSqr = glm::length2(a);
    float bPosSqr = glm::length2(b);
    float cPosSqr = glm::length2(c);
    float dPosSqr = glm::length2(d);

    glm::mat4 DxM(1.0f);
    DxM[0] = glm::vec4(aPosSqr, bPosSqr, cPosSqr, dPosSqr);
    DxM[1] = glm::vec4(a.y, b.y, c.y, d.y);
    DxM[2] = glm::vec4(a.z, b.z, c.z, d.z);
    DxM[3] = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
    float Dx = glm::determinant(DxM);

    glm::mat4 DyM(1.0f);
    DyM[0] = glm::vec4(aPosSqr, bPosSqr, cPosSqr, dPosSqr);
    DyM[1] = glm::vec4(a.x, b.x, c.x, d.x);
    DyM[2] = glm::vec4(a.z, b.z, c.z, d.z);
    DyM[3] = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
    float Dy = -glm::determinant(DyM);

    glm::mat4 DzM(1.0f);
    DzM[0] = glm::vec4(aPosSqr, bPosSqr, cPosSqr, dPosSqr);
    DzM[1] = glm::vec4(a.x, b.x, c.x, d.x);
    DzM[2] = glm::vec4(a.y, b.y, c.y, d.y);
    DzM[3] = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
    float Dz = glm::determinant(DzM);

    glm::mat4 cM(1.0f);
    cM[0] = glm::vec4(aPosSqr, bPosSqr, cPosSqr, dPosSqr);
    cM[1] = glm::vec4(a.x, b.x, c.x, d.x);
    cM[2] = glm::vec4(a.y, b.y, c.y, d.y);
    cM[3] = glm::vec4(a.z, b.z, c.z, d.z);
    float cD = glm::determinant(cM);

    circumCenter = glm::vec3(Dx / (2.0f * aD), Dy / (2.0f * aD), Dz / (2.0f * aD));
    circumRadius2 = ((Dx * Dx) + (Dy * Dy) + (Dz * Dz) - (4.0f * aD * cD)) / (4.0f * aD * aD);
}

bool Tetrahedron::CircumSphereContains(const glm::vec3 & pos)
{
	glm::vec3 diff = pos - circumCenter;
	return glm::length2(diff) <= circumRadius2;
}

bool Tetrahedron::ContainsVertex(const glm::vec3 & v)
{
	return DungeonGenUtils::AlmostEqual(a, v)
		|| DungeonGenUtils::AlmostEqual(b, v)
		|| DungeonGenUtils::AlmostEqual(c, v)
		|| DungeonGenUtils::AlmostEqual(d, v);
}

bool Tetrahedron::operator==(const Tetrahedron& t) const
{
	return a == t.a && b == t.b && c == t.c && d == t.d;
}

//////////////////////
// Triangle
//////////////////////

DelaunayTriangle::DelaunayTriangle(const glm::vec3& a, const glm::vec3& b, const glm::vec3& c)
	: a(a), b(b), c(c)
{}

bool DelaunayTriangle::AlmostEquals(const DelaunayTriangle& other)
{
	return (DungeonGenUtils::AlmostEqual(a, other.a) || DungeonGenUtils::AlmostEqual(a, other.b) || DungeonGenUtils::AlmostEqual(a, other.c))
		&& (DungeonGenUtils::AlmostEqual(b, other.a) || DungeonGenUtils::AlmostEqual(b, other.b) || DungeonGenUtils::AlmostEqual(b, other.c))
		&& (DungeonGenUtils::AlmostEqual(c, other.a) || DungeonGenUtils::AlmostEqual(c, other.b) || DungeonGenUtils::AlmostEqual(c, other.c));
}

bool DelaunayTriangle::operator==(const DelaunayTriangle& t) const
{
	return a == t.a && b == t.b && c == t.c;
}

bool DelaunayTriangle::CircumCircleContains(const glm::vec3& v) const
{
    float ab = glm::length2(a);
    float cd = glm::length2(b);
    float ef = glm::length2(c);

    float circumX = (ab * (c.y - b.y) + cd * (a.y - c.y) + ef * (b.y - a.y)) / (a.x * (c.y - b.y) + b.x * (a.y - c.y) + c.x * (b.y - a.y));
    float circumY = (ab * (c.x - b.x) + cd * (a.x - c.x) + ef * (b.x - a.x)) / (a.y * (c.x - b.x) + b.y * (a.x - c.x) + c.y * (b.x - a.x));

    glm::vec3 circum(circumX / 2.0f, v.y, circumY / 2.0f);
    float r2 = glm::length2(a - circum);
    float dist2 = glm::length2(v - circum);
    return dist2 <= r2;
}

bool DelaunayTriangle::ContainsVertex(const glm::vec3& v) const
{
    return glm::distance(v, a) < 0.01f
        || glm::distance(v, b) < 0.01f
        || glm::distance(v, c) < 0.01f;
}

//////////////////////
// Triangle 2D
//////////////////////

DelaunayTriangle2D::DelaunayTriangle2D(const glm::vec2& a, const glm::vec2& b, const glm::vec2& c)
    : a(a), b(b), c(c)
{}

bool DelaunayTriangle2D::AlmostEquals(const DelaunayTriangle2D& other)
{
    return (DungeonGenUtils::AlmostEqual(a, other.a) || DungeonGenUtils::AlmostEqual(a, other.b) || DungeonGenUtils::AlmostEqual(a, other.c))
        && (DungeonGenUtils::AlmostEqual(b, other.a) || DungeonGenUtils::AlmostEqual(b, other.b) || DungeonGenUtils::AlmostEqual(b, other.c))
        && (DungeonGenUtils::AlmostEqual(c, other.a) || DungeonGenUtils::AlmostEqual(c, other.b) || DungeonGenUtils::AlmostEqual(c, other.c));
}

bool DelaunayTriangle2D::operator==(const DelaunayTriangle2D& t) const
{
    return a == t.a && b == t.b && c == t.c;
}

bool DelaunayTriangle2D::CircumCircleContains(const glm::vec2& v) const
{
    float ab = glm::length2(a);
    float cd = glm::length2(b);
    float ef = glm::length2(c);

    float circumX = (ab * (c.y - b.y) + cd * (a.y - c.y) + ef * (b.y - a.y)) / (a.x * (c.y - b.y) + b.x * (a.y - c.y) + c.x * (b.y - a.y));
    float circumY = (ab * (c.x - b.x) + cd * (a.x - c.x) + ef * (b.x - a.x)) / (a.y * (c.x - b.x) + b.y * (a.x - c.x) + c.y * (b.x - a.x));

    glm::vec2 circum(circumX / 2.0f, circumY / 2.0f);
    float r2 = glm::length2(a - circum);
    float dist2 = glm::length2(v - circum);
    return dist2 <= r2;
}

bool DelaunayTriangle2D::ContainsVertex(const glm::vec2& v) const
{
    return glm::distance(v, a) < 0.01f
        || glm::distance(v, b) < 0.01f
        || glm::distance(v, c) < 0.01f;
}

//////////////////////
// Edge
//////////////////////

Edge::Edge(const glm::vec3& a, const glm::vec3& b)
	: a(a), b(b)
{}

bool Edge::operator==(const Edge& t) const
{
	return a == t.a && b == t.b;
}

bool Edge::AlmostEqual(const Edge& e) const
{
    return DungeonGenUtils::AlmostEqual(a, e.a) && DungeonGenUtils::AlmostEqual(b, e.b)
        || DungeonGenUtils::AlmostEqual(a, e.b) && DungeonGenUtils::AlmostEqual(b, e.a);
}

//////////////////////
// Edge 2D
//////////////////////

Edge2D::Edge2D(const glm::vec2& a, const glm::vec2& b)
    : a(a), b(b)
{}

bool Edge2D::operator==(const Edge2D& t) const
{
    return a == t.a && b == t.b;
}

bool Edge2D::AlmostEqual(const Edge2D& e) const
{
    return DungeonGenUtils::AlmostEqual(a, e.a) && DungeonGenUtils::AlmostEqual(b, e.b)
        || DungeonGenUtils::AlmostEqual(a, e.b) && DungeonGenUtils::AlmostEqual(b, e.a);
}

/*
stolen from:
https://github.com/299299/NagaGame/blob/master/Source/Game/Engine/Graphics/DebugDraw.h
*/
#pragma once
#include "render.h"

struct Frustum;
struct DebugLine;
struct DebugText;
struct ShaderProgram;
struct hkQsTransform;

struct DebugDrawManager
{
    void init( bgfx::ProgramHandle shader );
    void shutdown();

	void draw();

    void frame_start();
    void add_line(const float* start, const float* end, uint32_t color, bool bDepth);
    void add_aabb(const float* min, const float* max, uint32_t color, bool bDepth);
    void add_axis(const hkQsTransform& t, float size = 0.25f, bool bDepth = false);
    void add_cross(const float* pos, float size, uint32_t color, bool bDepth);
    void add_text_3d(const float* pos, const char* text, uint32_t color);
    void add_sphere(const float* pos, float radius, uint32_t color, bool bDepth);
    void add_cycle(const float* pos, const float* normal, float raidus, uint32_t color, bool bDepth);
    void add_triangle(const float* v0, const float* v1, const float* v2, uint32_t color, bool bDepth);
    void add_quad(const float* center, float width, float height, uint32_t color, bool bDepth);
    void add_frustum(const Frustum& frustum, uint32_t color, bool bDepth);
    void add_grid(int gridsNum, float gridWidth, uint32_t color, bool bDepth);
    void add_direction(const hkQsTransform& t, float len, uint32_t color, bool bDepth);

    DebugText*              m_texts;
    DebugLine*              m_lines[2];

    int                     m_numLines[2];
    int                     m_numTexts;

    bgfx::ProgramHandle     m_shader;
    bool                    m_ready;

private:
    void draw(int lineNum, DebugLine* lines, bool bDepth);
};

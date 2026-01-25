//-----------------------------------------------------------------------------
// File : MiscGL.cpp
// Desc : OpenGL Utility.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <MiscGL.h>
#include <cstdio>
#include <GL/glew.h>


//-----------------------------------------------------------------------------
//      OpenGLエラーがあるかどうかチェックします.
//-----------------------------------------------------------------------------
bool HasGLError()
{
    auto err = glGetError();
    if (err == GL_NO_ERROR)
    {
        // エラーなし.
        return false;
    }

    do {
        const char* msg = "";
        switch(err)
        {
        case GL_INVALID_ENUM:
            msg = "INVALID_ENUM";
            break;

        case GL_INVALID_VALUE:
            msg = "INVALID_VALUE";
            break;

        case GL_INVALID_OPERATION:
            msg = "INVALID_OPERATION";
            break;

        case GL_INVALID_FRAMEBUFFER_OPERATION:
            msg = "INVALID_FRAME_BUFFER_OPERATION";
            break;

        case GL_OUT_OF_MEMORY:
            msg = "OUT_OF_MEMORY";
            break;

        case GL_STACK_OVERFLOW:
            msg = "STACK_OVERFLOW";
            break;

        case GL_STACK_UNDERFLOW:
            msg = "STACK_UNDERFLOW";
            break;

        default:
            msg = "Unknown";
            break;
        }

        // エラーメッセージを取得.
        fprintf_s(stderr, "GL Error : errcode = 0x%x, msg = %s\n", err, msg);

        // 次のエラーメッセージを取得.
        err = glGetError();

    } while (err != GL_NO_ERROR);   // エラーが無くなるまで続行.

    // エラー有り.
    return true;
}
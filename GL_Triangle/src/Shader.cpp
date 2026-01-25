//-----------------------------------------------------------------------------
// File : Shader.cpp
// Desc : Shader Module.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include "Shader.h"
#include <vector>
#include <GL/glew.h>


namespace {

//-----------------------------------------------------------------------------
//      シェーダタイプを取得します.
//-----------------------------------------------------------------------------
GLenum GetShaderType(Shader::Kind type)
{
    switch(type)
    {
    case Shader::Kind::VS:
    default:
        return GL_VERTEX_SHADER;

    case Shader::Kind::PS:
        return GL_FRAGMENT_SHADER;

    case Shader::Kind::GS:
        return GL_GEOMETRY_SHADER;

    case Shader::Kind::DS:
        return GL_TESS_EVALUATION_SHADER;

    case Shader::Kind::HS:
        return GL_TESS_CONTROL_SHADER;

    case Shader::Kind::CS:
        return GL_COMPUTE_SHADER;
    }
}

} // namespace


///////////////////////////////////////////////////////////////////////////////
// Shader class
///////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
//      コンストラクタです.
//-----------------------------------------------------------------------------
Shader::Shader()
: m_Shader(0)
{ /* DO_NOTHING */ }

//-----------------------------------------------------------------------------
//      デストラクタです.
//-----------------------------------------------------------------------------
Shader::~Shader()
{ Term(); }

//-----------------------------------------------------------------------------
//      初期化処理を行います.
//-----------------------------------------------------------------------------
bool Shader::Init(Kind kind, const void* pBinary, uint32_t binarySize, const char* entryPoint)
{
    // シェーダオブジェクトを生成.
    auto type = GetShaderType(kind);
    m_Shader = glCreateShader(type);
    if (m_Shader == 0)
    { return false; }

    // シェーダバイナリを設定.
    glShaderBinary(1, &m_Shader, GL_SHADER_BINARY_FORMAT_SPIR_V, pBinary, binarySize);

    // エントリーポイントを設定.
    glSpecializeShader(m_Shader, entryPoint, 0, nullptr, nullptr);

    // 正常終了.
    return true;
}

//-----------------------------------------------------------------------------
//      ファイルから初期化します.
//-----------------------------------------------------------------------------
bool Shader::InitFromFile(Kind kind, const char* filename, const char* entryPoint)
{
    // ファイルを開く.
    FILE* fp = nullptr;
    auto err = fopen_s(&fp, filename, "rb");
    if (err != 0)
    { return false; }

    // ファイルサイズを求める.
    auto curPos = ftell(fp);
    fseek(fp, SEEK_END, 0);
    auto endPos = ftell(fp);
    auto size = endPos - curPos;
    fseek(fp, SEEK_SET, 0);

    // ファイルを一気に読み込む.
    std::vector<uint8_t> bin;
    bin.resize(size);
    fread(bin.data(), size, 1, fp);

    // ファイルを閉じる.
    fclose(fp);

    // 初期化処理を実行.
    return Init(kind, bin.data(), uint32_t(bin.size()), entryPoint);
}

//-----------------------------------------------------------------------------
//      終了処理を行います.
//-----------------------------------------------------------------------------
void Shader::Term()
{
    // シェーダオブジェクトを破棄.
    if (m_Shader != 0)
    {
        glDeleteShader(m_Shader);
        m_Shader = 0;
    }
}

//-----------------------------------------------------------------------------
//      シェーダを取得します.
//-----------------------------------------------------------------------------
uint32_t Shader::Get() const
{ return m_Shader; }


///////////////////////////////////////////////////////////////////////////////
// ShaderProgram class
///////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
//      コンストラクタです.
//-----------------------------------------------------------------------------
ShaderProgram::ShaderProgram()
: m_Program(0)
{ /* DO_NOTHING */ }

//-----------------------------------------------------------------------------
//      デストラクタです.
//-----------------------------------------------------------------------------
ShaderProgram::~ShaderProgram()
{ Term(); }

//-----------------------------------------------------------------------------
//      初期化処理を行います.
//-----------------------------------------------------------------------------
bool ShaderProgram::Init(const Shader** shaders, size_t shaderCount)
{
    // プログラムオブジェクトを生成.
    m_Program = glCreateProgram();
    if (m_Program == 0)
    { return false; }

    // シェーダをアタッチします.
    for(size_t i=0u; i<shaderCount; ++i)
    {
        auto obj = shaders[i]->Get();
        if (obj == 0)
            continue;

        glAttachShader(m_Program, obj);
    }

    // 正常終了.
    return Link();
}

//-----------------------------------------------------------------------------
//      終了処理を行います.
//-----------------------------------------------------------------------------
void ShaderProgram::Term()
{
    // プログラムオブジェクトを破棄.
    if (m_Program != 0)
    {
        glDeleteProgram(m_Program);
        m_Program = 0;
    }
}

//-----------------------------------------------------------------------------
//      リンクします.
//-----------------------------------------------------------------------------
bool ShaderProgram::Link()
{
    // プログラムをリンクします.
    glLinkProgram(m_Program);

    // リンク結果を取得します.
    GLint status = 0;
    glGetProgramiv(m_Program, GL_LINK_STATUS, &status);
    if (status == GL_FALSE)
    {
        // ログの長さを取得.
        GLsizei bufSize = 0;
        glGetProgramiv(m_Program, GL_INFO_LOG_LENGTH, &bufSize);
        if (bufSize > 0)
        {
            // ログを取得.
            std::vector<GLchar> log(bufSize);
            GLsizei length = 0;
            glGetProgramInfoLog(m_Program, bufSize, &length, &log[0]);

            // 標準エラー出力に表示させる.
            fprintf_s(stderr, "Error : Shader Program Link Error. msg = %s.", log.data());
        }

        // 異常終了.
        return false;
    }

    // 正常終了.
    return true;
}

//-----------------------------------------------------------------------------
//      バインドします.
//-----------------------------------------------------------------------------
void ShaderProgram::Bind()
{ glUseProgram(m_Program); }

//-----------------------------------------------------------------------------
//      バインドを解除します.
//-----------------------------------------------------------------------------
void ShaderProgram::Unbind()
{ glUseProgram(0); }

//-----------------------------------------------------------------------------
//      シェーダプログラムを取得します.
//-----------------------------------------------------------------------------
uint32_t ShaderProgram::Get() const
{ return m_Program; }

//-----------------------------------------------------------------------------
//      頂点属性を取得します.
//-----------------------------------------------------------------------------
int ShaderProgram::GetAttribute(const char* name) const
{
    if (m_Program == 0)
        return -1;

    return glGetAttribLocation(m_Program, name);
}

//-----------------------------------------------------------------------------
//      ユニフォームロケーションを取得します.
//-----------------------------------------------------------------------------
int ShaderProgram::GetUniform(const char* name) const
{
    if (m_Program == 0)
        return -1;

    return glGetUniformLocation(m_Program, name);
}
//-----------------------------------------------------------------------------
// File : Texture.cpp
// Desc : Texture Object.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------

#define STB_IMAGE_IMPLEMENTATION

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <Texture.h>
#include <GL/glew.h>
#include <stb_image.h>


///////////////////////////////////////////////////////////////////////////////
// Texture class
///////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
//      コンストラクタです.
//-----------------------------------------------------------------------------
Texture::Texture()
: m_Handle  (0)
, m_Desc    ()
{ /* DO_NOTHING */ }

//-----------------------------------------------------------------------------
//      デストラクタです.
//-----------------------------------------------------------------------------
Texture::~Texture()
{ Term(); }

//-----------------------------------------------------------------------------
//      初期化処理を行います.
//-----------------------------------------------------------------------------
bool Texture::Init(const Desc& desc, const void* pPixels)
{
    glCreateTextures(GL_TEXTURE_2D, 1, &m_Handle);
    if (m_Handle == 0)
        return false;

    glTextureStorage2D(
        m_Handle,
        1,
        desc.InternalFormat,
        desc.Width,
        desc.Height);

    glTextureSubImage2D(
        m_Handle,
        0,
        0,
        0,
        desc.Width,
        desc.Height,
        desc.Format,
        desc.Type,
        pPixels);

    return true;
}

//-----------------------------------------------------------------------------
//      ファイルから初期化処理を行います.
//-----------------------------------------------------------------------------
bool Texture::InitFromFile(const char* path)
{
    int w = 0;
    int h = 0;
    int c = 0;

    auto pixels = stbi_load(path, &w, &h, &c, STBI_default);
    if (!pixels)
        return false;

    GLenum format         = GL_RGBA;
    GLenum internalFormat = GL_RGBA8;

    switch (c)
    {
    case 1:
        format = GL_RED;
        internalFormat = GL_R8;
        break;
    case 2:
        format = GL_RG;
        internalFormat = GL_RG8;
        break;
    case 3:
        format = GL_RGB;
        internalFormat = GL_RGB8;
        break;
    case 4:
        format = GL_RGBA;
        internalFormat = GL_RGBA8;
        break;
    default:
        stbi_image_free(pixels);
        return false;
    }

    Desc desc{};
    desc.Width              = w;
    desc.Height             = h;
    desc.InternalFormat     = internalFormat;
    desc.Format             = format;
    desc.Type               = GL_UNSIGNED_BYTE;

    auto result = Init(desc, pixels);

    stbi_image_free(pixels);

    return result;
}

//-----------------------------------------------------------------------------
//      終了処理を行います.
//-----------------------------------------------------------------------------
void Texture::Term()
{
    if (m_Handle)
    {
        glDeleteTextures(1, &m_Handle);
        m_Handle = 0;
    }

    m_Desc = {};
}

//-----------------------------------------------------------------------------
//      バインドします.
//-----------------------------------------------------------------------------
void Texture::Bind(int binding)
{ glBindTextureUnit(binding, m_Handle); }

//-----------------------------------------------------------------------------
//      バインドを解除します.
//-----------------------------------------------------------------------------
void Texture::Unbind(int binding)
{ glBindTextureUnit(binding, 0); }

//-----------------------------------------------------------------------------
//      テクスチャハンドルを取得します.
//-----------------------------------------------------------------------------
uint32_t Texture::GetHandle() const
{ return m_Handle; }

//-----------------------------------------------------------------------------
//      構成設定を取得します.
//-----------------------------------------------------------------------------
const Texture::Desc& Texture::GetDesc() const
{ return m_Desc; }


///////////////////////////////////////////////////////////////////////////////
// Sampler class
///////////////////////////////////////////////////////////////////////////////
const Sampler::Desc Sampler::PointWrap = { 
    GL_NEAREST,
    GL_NEAREST,
    GL_REPEAT,
    GL_REPEAT,
    GL_REPEAT,
    { 1.0f, 1.0f, 1.0f, 1.0f },
    0.0f,
    GL_NEVER,
};
const Sampler::Desc Sampler::PointClamp = {
    GL_NEAREST,
    GL_NEAREST,
    GL_CLAMP,
    GL_CLAMP,
    GL_CLAMP,
    { 1.0f, 1.0f, 1.0f, 1.0f },
    0.0f,
    GL_NEVER
};
const Sampler::Desc Sampler::PointMirror = {
    GL_NEAREST,
    GL_NEAREST,
    GL_MIRRORED_REPEAT,
    GL_MIRRORED_REPEAT,
    GL_MIRRORED_REPEAT,
    { 1.0f, 1.0f, 1.0f, 1.0f },
    0.0f,
    GL_NEVER
};
const Sampler::Desc Sampler::LinearWrap = {
    GL_LINEAR,
    GL_LINEAR,
    GL_REPEAT,
    GL_REPEAT,
    GL_REPEAT,
    { 1.0f, 1.0f, 1.0f, 1.0f },
    0.0f,
    GL_NEVER
};
const Sampler::Desc Sampler::LinearClamp = {
    GL_LINEAR,
    GL_LINEAR,
    GL_CLAMP,
    GL_CLAMP,
    GL_CLAMP,
    { 1.0f, 1.0f, 1.0f, 1.0f },
    0.0f,
    GL_NEVER
};
const Sampler::Desc Sampler::LinearMirror = {
    GL_LINEAR,
    GL_LINEAR,
    GL_MIRRORED_REPEAT,
    GL_MIRRORED_REPEAT,
    GL_MIRRORED_REPEAT,
    { 1.0f, 1.0f, 1.0f, 1.0f },
    0.0f,

};

//-----------------------------------------------------------------------------
//      コンストラクタです.
//-----------------------------------------------------------------------------
Sampler::Sampler()
: m_Handle(0)
{ /* DO_NOTHING */ }

//-----------------------------------------------------------------------------
//      デストラクタです.
//-----------------------------------------------------------------------------
Sampler::~Sampler()
{ Term(); }

//-----------------------------------------------------------------------------
//      初期化処理を行います.
//-----------------------------------------------------------------------------
bool Sampler::Init(const Desc& desc)
{
    glCreateSamplers(1, &m_Handle);
    if (m_Handle == 0)
        return false;

    glSamplerParameteri (m_Handle, GL_TEXTURE_MIN_FILTER,   desc.MinFilter);
    glSamplerParameteri (m_Handle, GL_TEXTURE_MAG_FILTER,   desc.MagFilter);
    glSamplerParameteri (m_Handle, GL_TEXTURE_WRAP_S,       desc.AddressU);
    glSamplerParameteri (m_Handle, GL_TEXTURE_WRAP_T,       desc.AddressV);
    glSamplerParameteri (m_Handle, GL_TEXTURE_WRAP_R,       desc.AddressW);
    glSamplerParameterfv(m_Handle, GL_TEXTURE_BORDER_COLOR, desc.BorderColor);
    glSamplerParameterf (m_Handle, GL_TEXTURE_LOD_BIAS,     desc.LodBias);
    glSamplerParameteri (m_Handle, GL_TEXTURE_COMPARE_FUNC, desc.CompareFunc);

    return true;
}

//-----------------------------------------------------------------------------
//      終了処理をおこないます.
//-----------------------------------------------------------------------------
void Sampler::Term()
{
    if (m_Handle)
    {
        glDeleteSamplers(1, &m_Handle);
        m_Handle = 0;
    }
}

//-----------------------------------------------------------------------------
//      バインドをします.
//-----------------------------------------------------------------------------
void Sampler::Bind(int binding)
{ glBindSampler(binding, m_Handle); }

//-----------------------------------------------------------------------------
//      バインドを解除します.
//-----------------------------------------------------------------------------
void Sampler::Unbind(int binding)
{ glBindSampler(binding, 0); }

//-----------------------------------------------------------------------------
//      サンプラーオブジェクトを取得します.
//-----------------------------------------------------------------------------
uint32_t Sampler::GetHandle() const
{ return m_Handle; }

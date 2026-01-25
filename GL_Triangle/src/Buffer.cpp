//----------------------------------------------------------------------------
// File : Buffer.h
// Desc : Buffer Objects.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <GL/glew.h>
#include <Buffer.h>


///////////////////////////////////////////////////////////////////////////////
// VertexBuffer class
///////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
//      コンストラクタです.
//-----------------------------------------------------------------------------
VertexBuffer::VertexBuffer()
: m_BufferObject(0)
, m_VertexCount (0)
, m_VertexStride(0)
{ /* DO_NOTHING */ }

//-----------------------------------------------------------------------------
//      デストラクタです.
//-----------------------------------------------------------------------------
VertexBuffer::~VertexBuffer()
{ Term(); }

//-----------------------------------------------------------------------------
//      初期化処理を行います.
//-----------------------------------------------------------------------------
bool VertexBuffer::Init(const void* vertices, uint32_t vertexCount, uint32_t vertexStride)
{
    glGenBuffers(1, &m_BufferObject);
    if (m_BufferObject == 0)
    { return false; }

    auto bufferSize = vertexCount * vertexStride;

    glBindBuffer(GL_ARRAY_BUFFER, m_BufferObject);
    glBufferData(GL_ARRAY_BUFFER, bufferSize, vertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    m_VertexCount  = vertexCount;
    m_VertexStride = vertexStride;

    return true;
}

//-----------------------------------------------------------------------------
//      終了処理を行います.
//-----------------------------------------------------------------------------
void VertexBuffer::Term()
{
    if (m_BufferObject)
    {
        glDeleteBuffers(1, &m_BufferObject);
        m_BufferObject = 0;
    }

    m_VertexCount  = 0;
    m_VertexStride = 0;
}

//-----------------------------------------------------------------------------
//      バッファオブジェクトを取得します.
//-----------------------------------------------------------------------------
uint32_t VertexBuffer::GetBufferObject() const
{ return m_BufferObject; }

//-----------------------------------------------------------------------------
//      頂点数を取得します.
//-----------------------------------------------------------------------------
uint32_t VertexBuffer::GetVertexCount() const
{ return m_VertexCount; }

//-----------------------------------------------------------------------------
//      1頂点あたりのデータサイズを取得します.
//-----------------------------------------------------------------------------
uint32_t VertexBuffer::GetVertexStride() const
{ return m_VertexStride; }

//-----------------------------------------------------------------------------
//      バッファオブジェクトをバインドします.
//-----------------------------------------------------------------------------
void VertexBuffer::Bind(const InputLayout& inputLayout)
{
    glBindBuffer(GL_ARRAY_BUFFER, m_BufferObject);

    if (inputLayout.pAttributes != nullptr)
    {
        for(auto i=0u; i<inputLayout.AttributeCount; ++i)
        {
            const auto& attr = inputLayout.pAttributes[i];
            glEnableVertexAttribArray(attr.Location);
            glVertexAttribPointer(
                attr.Location,
                attr.Count, 
                attr.DataType,
                attr.Normalized,
                m_VertexStride,
                (const void*)attr.Offset); 
        }
    }
}

//-----------------------------------------------------------------------------
//      バッファオブジェクトのバインドを解除します.
//-----------------------------------------------------------------------------
void VertexBuffer::Unbind(const InputLayout& inputLayout)
{
    if (inputLayout.pAttributes != nullptr)
    {
        for(auto i=0u; i<inputLayout.AttributeCount; ++i)
        {
            const auto& attr = inputLayout.pAttributes[i];
            glDisableVertexAttribArray(attr.Location);
        }
    }
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}


///////////////////////////////////////////////////////////////////////////////
// IndexBuffer class
///////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
//      コンストラクタです.
//-----------------------------------------------------------------------------
IndexBuffer::IndexBuffer()
: m_BufferObject(0)
, m_IndexCount  (0)
{ /* DO_NOTHING */ }

//-----------------------------------------------------------------------------
//      デストラクタです.
//-----------------------------------------------------------------------------
IndexBuffer::~IndexBuffer()
{ Term(); }

//-----------------------------------------------------------------------------
//      初期化処理を行います.
//-----------------------------------------------------------------------------
bool IndexBuffer::Init(const uint32_t* indices, uint32_t indexCount)
{
    glGenBuffers(1, &m_BufferObject);
    if (m_BufferObject == 0)
    { return false; }

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_BufferObject);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexCount * sizeof(uint32_t), indices, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    m_IndexCount = indexCount;

    return true;
}

//-----------------------------------------------------------------------------
//      終了処理を行います.
//-----------------------------------------------------------------------------
void IndexBuffer::Term()
{
    if (m_BufferObject)
    {
        glDeleteBuffers(1, &m_BufferObject);
        m_BufferObject = 0;
    }

    m_IndexCount = 0;
}

//-----------------------------------------------------------------------------
//      バッファオブジェクトを取得します.
//-----------------------------------------------------------------------------
uint32_t IndexBuffer::GetBufferObject() const
{ return m_BufferObject; }

//-----------------------------------------------------------------------------
//      インデックス数を取得します.
//-----------------------------------------------------------------------------
uint32_t IndexBuffer::GetIndexCount() const
{ return m_IndexCount; }

//-----------------------------------------------------------------------------
//      バッファオブジェクトをバインドします.
//-----------------------------------------------------------------------------
void IndexBuffer::Bind()
{ glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_BufferObject); }

//-----------------------------------------------------------------------------
//      バッファオブジェクトのバインドを解除します.
//-----------------------------------------------------------------------------
void IndexBuffer::Unbind()
{ glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0); }


///////////////////////////////////////////////////////////////////////////////
// UniformBuffer class
///////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
//      コンストラクタです.
//-----------------------------------------------------------------------------
UniformBuffer::UniformBuffer()
: m_BufferObject(0)
{ /* DO_NOTHING */ }

//-----------------------------------------------------------------------------
//      デストラクタです.
//-----------------------------------------------------------------------------
UniformBuffer::~UniformBuffer()
{ Term(); }

//-----------------------------------------------------------------------------
//      初期化処理を行います.
//-----------------------------------------------------------------------------
bool UniformBuffer::Init(const void* buffer, uint32_t bufferSize)
{
    glGenBuffers(1, &m_BufferObject);
    if (m_BufferObject == 0)
    { return false; }

    glBindBuffer(GL_UNIFORM_BUFFER, m_BufferObject);
    glBufferData(GL_UNIFORM_BUFFER, bufferSize, buffer, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    return true;
}

//-----------------------------------------------------------------------------
//      終了処理を行います.
//-----------------------------------------------------------------------------
void UniformBuffer::Term()
{
    if (m_BufferObject)
    {
        glDeleteBuffers(1, &m_BufferObject);
        m_BufferObject = 0;
    }
}

//-----------------------------------------------------------------------------
//      更新処理を行います.
//-----------------------------------------------------------------------------
void UniformBuffer::Update(const void* data, uint32_t size, uint32_t offset)
{
    if (m_BufferObject == 0)
        return;

    glBindBuffer(GL_UNIFORM_BUFFER, m_BufferObject);
    glBufferSubData(GL_UNIFORM_BUFFER, offset, size, data);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

//-----------------------------------------------------------------------------
//      バッファオブジェクトを取得します.
//-----------------------------------------------------------------------------
uint32_t UniformBuffer::GetBufferObject() const
{ return m_BufferObject; }

//-----------------------------------------------------------------------------
//      バッファオブジェクトをバインドします.
//-----------------------------------------------------------------------------
void UniformBuffer::Bind(int binding)
{ glBindBufferBase(GL_UNIFORM_BUFFER, binding, m_BufferObject); }

//-----------------------------------------------------------------------------
//      バッファオブジェクトのバインドを解除します.
//-----------------------------------------------------------------------------
void UniformBuffer::Unbind(int binding)
{ glBindBufferBase(GL_UNIFORM_BUFFER, binding, 0); }


///////////////////////////////////////////////////////////////////////////////
// StorageBuffer class
///////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
//      コンストラクタです.
//-----------------------------------------------------------------------------
StorageBuffer::StorageBuffer()
: m_BufferObject(0)
{ /* DO_NOTHING */ }

//-----------------------------------------------------------------------------
//      デストラクタです.
//-----------------------------------------------------------------------------
StorageBuffer::~StorageBuffer()
{ Term(); }

//-----------------------------------------------------------------------------
//      初期化処理を行います.
//-----------------------------------------------------------------------------
bool StorageBuffer::Init(const void* buffer, uint32_t bufferSize)
{
    glGenBuffers(1, &m_BufferObject);
    if (m_BufferObject == 0)
    { return false; }

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_BufferObject);
    glBufferData(GL_SHADER_STORAGE_BUFFER, bufferSize, buffer, GL_STATIC_DRAW);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

    return true;}

//-----------------------------------------------------------------------------
//      終了処理を行います.
//-----------------------------------------------------------------------------
void StorageBuffer::Term()
{
    if (m_BufferObject)
    {
        glDeleteBuffers(1, &m_BufferObject);
        m_BufferObject = 0;
    }
}

//-----------------------------------------------------------------------------
//      バッファオブジェクトを取得します.
//-----------------------------------------------------------------------------
uint32_t StorageBuffer::GetBufferObject() const
{ return m_BufferObject; }

//-----------------------------------------------------------------------------
//      バッファオブジェクトをバインドします.
//-----------------------------------------------------------------------------
void StorageBuffer::Bind(int binding)
{ glBindBufferBase(GL_SHADER_STORAGE_BUFFER, binding, m_BufferObject); }

//-----------------------------------------------------------------------------
//      バッファオブジェクトのバインドを解除します.
//-----------------------------------------------------------------------------
void StorageBuffer::Unbind(int binding)
{ glBindBufferBase(GL_SHADER_STORAGE_BUFFER, binding, 0); }

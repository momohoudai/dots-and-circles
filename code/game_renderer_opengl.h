#ifndef __RENDERER_OPENGL__
#define __RENDERER_OPENGL__


// Configuration
#define OPENGL_MAX_TEXTURES 8
#define OPENGL_MAX_ENTITIES 4096

// Opengl typedefs
#define GL_TRUE                 1
#define GL_FALSE                0

#define GL_DEPTH_TEST                   0x0B71
#define GL_SCISSOR_TEST                 0x0C11
#define GL_DEPTH_BUFFER_BIT             0x00000100
#define GL_COLOR_BUFFER_BIT             0x00004000
#define GL_DEBUG_OUTPUT                 0x92E0
#define GL_DEBUG_OUTPUT_SYNCHRONOUS     0x8242
#define GL_FLOAT                        0x1406
#define GL_DYNAMIC_STORAGE_BIT          0x0100
#define GL_TEXTURE_2D                   0x0DE1
#define GL_FRAGMENT_SHADER              0x8B30
#define GL_VERTEX_SHADER                0x8B31
#define GL_LINK_STATUS                  0x8B82
#define GL_BLEND                        0x0BE2
#define GL_SRC_ALPHA                    0x0302
#define GL_ONE_MINUS_SRC_ALPHA          0x0303
#define GL_RGBA                         0x1908
#define GL_RGBA8                        0x8058
#define GL_UNSIGNED_BYTE                0x1401
#define GL_TRIANGLES                    0x0004
#define GL_NEAREST                      0x2600
#define GL_LINEAR                       0x2601
#define GL_TEXTURE_MIN_FILTER           0x2801
#define GL_TEXTURE_MAG_FILTER           0x2800
#define GL_DEBUG_SOURCE_API 0x8246
#define GL_DEBUG_SOURCE_WINDOW_SYSTEM 0x8247
#define GL_DEBUG_SOURCE_SHADER_COMPILER 0x8248
#define GL_DEBUG_SOURCE_THIRD_PARTY 0x8249
#define GL_DEBUG_SOURCE_APPLICATION 0x824A
#define GL_DEBUG_SOURCE_OTHER 0x824B
#define GL_DEBUG_SEVERITY_HIGH 0x9146
#define GL_DEBUG_SEVERITY_MEDIUM 0x9147
#define GL_DEBUG_SEVERITY_LOW 0x9148
#define GL_DEBUG_SEVERITY_NOTIFICATION 0x826B
#define GL_DEBUG_TYPE_ERROR 0x824C
#define GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR 0x824D
#define GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR 0x824E
#define GL_DEBUG_TYPE_PORTABILITY 0x824F
#define GL_DEBUG_TYPE_PERFORMANCE 0x8250
#define GL_DEBUG_TYPE_OTHER 0x8251
#define GL_DEBUG_TYPE_MARKER 0x8268

typedef s32  GLenum;
typedef s32  GLint; 
typedef s32  GLsizei;
typedef u32  GLuint;
typedef c8   GLchar;
typedef u32  GLbitfield;
typedef f32  GLclampf;
typedef smi  GLsizeiptr; 
typedef smi  GLintptr;
typedef b8   GLboolean;
typedef f32  GLfloat;

#define Opengl_DeclareOpenglFunction(Name) opengl_func_##Name
#define Opengl_DeclareOpenglPointer(Name) Opengl_DeclareOpenglFunction(Name)* Name
#define Opengl_DeclareOpenglCallbackFunction(Name) void Name(GLenum source,GLenum type,GLuint id,GLenum severity,GLsizei length,const GLchar *msg,const void *userParam)
typedef Opengl_DeclareOpenglCallbackFunction(GLDEBUGPROC);

typedef void    Opengl_DeclareOpenglFunction(glEnable)(GLenum cap);
typedef void    Opengl_DeclareOpenglFunction(glDisable)(GLenum cap);
typedef void    Opengl_DeclareOpenglFunction(glViewport)(GLint x, 
                                                         GLint y, 
                                                         GLsizei width, 
                                                         GLsizei height);
typedef void    Opengl_DeclareOpenglFunction(glScissor)(GLint x, 
                                                        GLint y, 
                                                        GLsizei width, 
                                                        GLsizei height); 
typedef GLuint  Opengl_DeclareOpenglFunction(glCreateShader)(GLenum type);
typedef void    Opengl_DeclareOpenglFunction(glCompileShader)(GLuint program);
typedef void    Opengl_DeclareOpenglFunction(glShaderSource)(GLuint shader, 
                                                             GLsizei count, 
                                                             GLchar** string, 
                                                             GLint* length);
typedef void    Opengl_DeclareOpenglFunction(glAttachShader)(GLuint program, GLuint shader);
typedef void    Opengl_DeclareOpenglFunction(glDeleteShader)(GLuint program);
typedef void    Opengl_DeclareOpenglFunction(glClear)(GLbitfield mask);
typedef void    Opengl_DeclareOpenglFunction(glClearColor)(GLclampf r, 
                                                           GLclampf g, 
                                                           GLclampf b, 
                                                           GLclampf a);
typedef void    Opengl_DeclareOpenglFunction(glCreateBuffers)(GLsizei n, GLuint* buffers);
typedef void    Opengl_DeclareOpenglFunction(glNamedBufferStorage)(GLuint buffer, 
                                                                   GLsizeiptr size, 
                                                                   const void* data, 
                                                                   GLbitfield flags);
typedef void    Opengl_DeclareOpenglFunction(glCreateVertexArrays)(GLsizei n, GLuint* arrays);
typedef void    Opengl_DeclareOpenglFunction(glVertexArrayVertexBuffer)(GLuint vaobj, 
                                                                        GLuint bindingindex, 
                                                                        GLuint buffer, 
                                                                        GLintptr offset, 
                                                                        GLsizei stride);
typedef void    Opengl_DeclareOpenglFunction(glEnableVertexArrayAttrib)(GLuint vaobj, GLuint index);
typedef void    Opengl_DeclareOpenglFunction(glVertexArrayAttribFormat)(GLuint vaobj,
                                                                        GLuint attribindex,
                                                                        GLint size,
                                                                        GLenum type,
                                                                        GLboolean normalized,
                                                                        GLuint relativeoffset);
typedef void    Opengl_DeclareOpenglFunction(glVertexArrayAttribBinding)(GLuint vaobj,
                                                                         GLuint attribindex,
                                                                         GLuint bindingindex);
typedef void    Opengl_DeclareOpenglFunction(glVertexArrayBindingDivisor)(GLuint vaobj,
                                                                          GLuint bindingindex,
                                                                          GLuint divisor);
typedef void    Opengl_DeclareOpenglFunction(glBlendFunc)(GLenum sfactor, GLenum dfactor);
typedef void    Opengl_DeclareOpenglFunction(glVertexArrayElementBuffer)(GLuint vaobj, GLuint buffer);
typedef GLuint  Opengl_DeclareOpenglFunction(glCreateProgram)();
typedef void    Opengl_DeclareOpenglFunction(glLinkProgram)(GLuint program);
typedef void    Opengl_DeclareOpenglFunction(glGetProgramiv)(GLuint program, GLenum pname, GLint* params);
typedef void    Opengl_DeclareOpenglFunction(glGetProgramInfoLog)(GLuint program, 
                                                                  GLsizei maxLength,
                                                                  GLsizei* length,
                                                                  GLchar* infoLog);
typedef void    Opengl_DeclareOpenglFunction(glCreateTextures)(GLenum target, 
                                                               GLsizei n, 
                                                               GLuint* textures);
typedef void    Opengl_DeclareOpenglFunction(glTextureStorage2D)(GLuint texture,
                                                                 GLsizei levels,
                                                                 GLenum internalformat,
                                                                 GLsizei width,
                                                                 GLsizei height);
typedef void    Opengl_DeclareOpenglFunction(glTextureSubImage2D)(GLuint texture,
                                                                  GLint level,
                                                                  GLint xoffset,
                                                                  GLint yoffset,
                                                                  GLsizei width,
                                                                  GLsizei height,
                                                                  GLenum format,
                                                                  GLenum type,
                                                                  const void* pixels);
typedef void    Opengl_DeclareOpenglFunction(glBindTexture)(GLenum target, GLuint texture);
typedef void    Opengl_DeclareOpenglFunction(glTexParameteri)(GLenum target ,GLenum pname, GLint param);
typedef void    Opengl_DeclareOpenglFunction(glBindVertexArray)(GLuint array);
typedef void    Opengl_DeclareOpenglFunction(glDrawElementsInstancedBaseInstance)(GLenum mode,
                                                                                  GLsizei count,
                                                                                  GLenum type,
                                                                                  const void* indices,
                                                                                  GLsizei instancecount,
                                                                                  GLuint baseinstance);
typedef void    Opengl_DeclareOpenglFunction(glUseProgram)(GLuint program);
typedef void    Opengl_DeclareOpenglFunction(glNamedBufferSubData)(GLuint buffer,
                                                                   GLintptr offset,
                                                                   GLsizeiptr size,
                                                                   const void* data);
typedef GLint   Opengl_DeclareOpenglFunction(glGetUniformLocation)(GLuint program,
                                                                   const GLchar* name);
typedef void    Opengl_DeclareOpenglFunction(glProgramUniformMatrix4fv)(GLuint program,
                                                                        GLint location,
                                                                        GLsizei count,
                                                                        GLboolean transpose,
                                                                        const GLfloat* value);
typedef void    Opengl_DeclareOpenglFunction(glUseProgram)(GLuint program);
typedef void    Opengl_DeclareOpenglFunction(glDeleteTextures)(GLsizei n, 
                                                               const GLuint* textures);
typedef void    Opengl_DeclareOpenglFunction(glDebugMessageCallbackARB)(GLDEBUGPROC *callback, 
                                                                        const void *userParam);


// NOTE(Momo): Buffers
enum Opengl__VBO {
    Opengl__VBO_Model,
    Opengl__VBO_Indices,
    Opengl__VBO_Colors,
    Opengl__VBO_Texture,
    Opengl__VBO_Transform,
    Opengl__VBO_Count
};


// NOTE(Momo): Attributes
enum  Opengl__ATB { 
    Opengl__ATB_Model,    // 0 
    Opengl__ATB_Colors,   // 1
    Opengl__ATB_Texture_1, // 2
    Opengl__ATB_Texture_2, // 3
    Opengl__ATB_Texture_3, // 4
    Opengl__ATB_Texture_4, // 5
    Opengl__ATB_Transform_1, // 6
    Opengl__ATB_Transform_2, // 7
    Opengl__ATB_Transform_3, // 8
    Opengl__ATB_Transform_4  // 9
};

// NOTE(Momo): VAO bindings
enum Opengl__VAO_Binding {
    Opengl__VAO_Binding_Model,
    Opengl__VAO_Binding_Colors,
    Opengl__VAO_Binding_Texture,
    Opengl__VAO_Binding_Transform
};

enum Opengl__Predefined_Texture_ID{
    Opengl__Predefined_Texture_ID_Dummy,
    Opengl__Predefined_Texture_ID_Blank,
};

struct Opengl : Renderer {
	
    constexpr static const char* vertex_shader = R"###(
    #version 450 core
    layout(location=0) in vec3 aModelVtx; 
    layout(location=1) in vec4 aColor;
    layout(location=2) in vec2 aTexCoord[4];
    layout(location=6) in mat4 aTransform;
    out vec4 mColor;
    out vec2 mTexCoord;
    uniform mat4 uProjection;
    
    void main(void) {
        gl_Position = uProjection * aTransform *  vec4(aModelVtx, 1.0);
        mColor = aColor;
        mTexCoord = aTexCoord[gl_VertexID];
        //mTexCoord.y = 1.0 - mTexCoord.y;
    })###";
    
    constexpr static const char* fragment_shader = R"###(
    #version 450 core
    out vec4 fragColor;
    in vec4 mColor;
    in vec2 mTexCoord;
    uniform sampler2D uTexture;
    
    void main(void) {
        fragColor = texture(uTexture, mTexCoord) * mColor; 
    })###";
    
    
    // Stuff to work with game
    constexpr static inline f32 quad_model[] = {
        -0.5f, -0.5f, 0.0f,  // bottom left
        0.5f, -0.5f, 0.0f,  // bottom right
        0.5f,  0.5f, 0.0f,  // top right
        -0.5f,  0.5f, 0.0f,   // top left 
    };
    
    constexpr static inline u8 quad_indices[] = {
        0, 1, 2,
        0, 2, 3,
    };
    
    constexpr static inline f32 quad_uv[] = {
        0.0f, 1.0f,  // top left
        1.0f, 1.0f, // top right
        1.0f, 0.f, // bottom right
        0.f, 0.f, // bottom left
    };
    
    // Bindings that needs to be filled by platform
    Opengl_DeclareOpenglPointer(glEnable);
    Opengl_DeclareOpenglPointer(glDisable); 
    Opengl_DeclareOpenglPointer(glViewport);
    Opengl_DeclareOpenglPointer(glScissor);
    Opengl_DeclareOpenglPointer(glCreateShader);
    Opengl_DeclareOpenglPointer(glCompileShader);
    Opengl_DeclareOpenglPointer(glShaderSource);
    Opengl_DeclareOpenglPointer(glAttachShader);
    Opengl_DeclareOpenglPointer(glDeleteShader);
    Opengl_DeclareOpenglPointer(glClear);
    Opengl_DeclareOpenglPointer(glClearColor);
    Opengl_DeclareOpenglPointer(glCreateBuffers);
    Opengl_DeclareOpenglPointer(glNamedBufferStorage);
    Opengl_DeclareOpenglPointer(glCreateVertexArrays);
    Opengl_DeclareOpenglPointer(glVertexArrayVertexBuffer);
    Opengl_DeclareOpenglPointer(glEnableVertexArrayAttrib);
    Opengl_DeclareOpenglPointer(glVertexArrayAttribFormat);
    Opengl_DeclareOpenglPointer(glVertexArrayAttribBinding);
    Opengl_DeclareOpenglPointer(glVertexArrayBindingDivisor);
    Opengl_DeclareOpenglPointer(glBlendFunc);
    Opengl_DeclareOpenglPointer(glCreateProgram);
    Opengl_DeclareOpenglPointer(glLinkProgram);
    Opengl_DeclareOpenglPointer(glGetProgramiv);
    Opengl_DeclareOpenglPointer(glGetProgramInfoLog);
    Opengl_DeclareOpenglPointer(glVertexArrayElementBuffer);
    Opengl_DeclareOpenglPointer(glCreateTextures);
    Opengl_DeclareOpenglPointer(glTextureStorage2D);
    Opengl_DeclareOpenglPointer(glTextureSubImage2D);
    Opengl_DeclareOpenglPointer(glBindTexture);
    Opengl_DeclareOpenglPointer(glTexParameteri);
    Opengl_DeclareOpenglPointer(glBindVertexArray);
    Opengl_DeclareOpenglPointer(glDrawElementsInstancedBaseInstance);
    Opengl_DeclareOpenglPointer(glGetUniformLocation);
    Opengl_DeclareOpenglPointer(glProgramUniformMatrix4fv);
    Opengl_DeclareOpenglPointer(glNamedBufferSubData);
    Opengl_DeclareOpenglPointer(glUseProgram);
    Opengl_DeclareOpenglPointer(glDeleteTextures);
    Opengl_DeclareOpenglPointer(glDebugMessageCallbackARB);
    
	
	Arena arena; // Boot it
	
    GLuint buffers[Opengl__VBO_Count]; 
    GLuint shader;
    
    // NOTE(Momo): We only need one blueprint which is a 1x1 square.
    GLuint model; 
    
    // NOTE(Momo): A table mapping  between
    // 'game texture handler' <-> 'opengl texture handler'  
    // Index 0 will always be an invalid 'dummy texture
    // Index 1 will always be a blank texture for items with no texture (but has colors)
    List<GLuint> textures;
    
	Rect2u render_region;
};

static inline void 
Opengl__AttachShader(Opengl* ogl, u32 program, u32 type, char* Code) {
    GLuint shader_handle = ogl->glCreateShader(type);
    ogl->glShaderSource(shader_handle, 1, &Code, NULL);
    ogl->glCompileShader(shader_handle);
    ogl->glAttachShader(program, shader_handle);
    ogl->glDeleteShader(shader_handle);
}

// TODO: Maybe this should be a command?
static inline void 
Opengl__AlignViewport(Opengl* ogl, Vec2u render_wh, Rect2u region) 
{
	
    u32 x, y, w, h;
    x = region.min.x;
    y = region.min.y;
    w = Rect_Width(region);
    h = Rect_Height(region);
    
	ogl->glScissor(0, 0, render_wh.w, render_wh.h);
	ogl->glViewport(0, 0, render_wh.w, render_wh.h);
    		
	ogl->glClearColor(0.f, 0.f, 0.f, 0.f);
	ogl->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	
    ogl->glScissor(x, y, w, h);
	ogl->glViewport(x, y, w, h);
}


void 
Opengl__AddPredefinedTextures(Opengl* ogl) {
    struct Pixel { u8 e[4]; };
    
    
    // NOTE(Momo): Dummy texture setup
    {
        Pixel pixels[4] = {
            { 125, 125, 125, 255 },
            { 255, 255, 255, 255 },
            { 255, 255, 255, 255 },
            { 125, 125, 125, 255 },
        };
        
        GLuint dummy_texture;
        ogl->glCreateTextures(GL_TEXTURE_2D, 1, &dummy_texture);
        ogl->glTextureStorage2D(dummy_texture, 1, GL_RGBA8, 2, 2);
        ogl->glTextureSubImage2D(dummy_texture, 
                                 0, 0, 0, 
                                 2, 2, 
                                 GL_RGBA, 
                                 GL_UNSIGNED_BYTE, 
                                 &pixels);
        List_PushItem(&ogl->textures, dummy_texture);
    }
    
    // NOTE(Momo): Blank texture setup
    {
        Pixel pixels = { 255, 255, 255, 255 };
        GLuint blank_texture;
        ogl->glCreateTextures(GL_TEXTURE_2D, 1, &blank_texture);
        ogl->glTextureStorage2D(blank_texture, 1, GL_RGBA8, 1, 1);
        ogl->glTextureSubImage2D(blank_texture, 
                                 0, 0, 0, 
                                 1, 1, 
                                 GL_RGBA, GL_UNSIGNED_BYTE, 
                                 &pixels);
        List_PushItem(&ogl->textures, blank_texture);
    }
    
    
}

static inline b8
Opengl_Init(Opengl* ogl, 
			u32 texture_count,
			u32 render_commands_size)
{	
	if (!List_Alloc(&ogl->textures, &ogl->arena, texture_count)) {
		return false;
	}
	
	
	if (!Mailbox_Alloc(&ogl->render_commands, &ogl->arena, render_commands_size)) {
		return false;
	}
	
	
    ogl->glEnable(GL_DEPTH_TEST);
    ogl->glEnable(GL_SCISSOR_TEST);
					 
					 
    
    // NOTE(Momo): Setup VBO
    ogl->glCreateBuffers(Opengl__VBO_Count, ogl->buffers);
    ogl->glNamedBufferStorage(ogl->buffers[Opengl__VBO_Model], 
                              sizeof(ogl->quad_model), 
                              ogl->quad_model, 
                              0);
    
    ogl->glNamedBufferStorage(ogl->buffers[Opengl__VBO_Indices], 
                              sizeof(ogl->quad_indices), 
                              ogl->quad_indices, 
                              0);
    
    ogl->glNamedBufferStorage(ogl->buffers[Opengl__VBO_Texture], 
                              sizeof(Vec2f) * 4 * OPENGL_MAX_ENTITIES, 
                              nullptr, 
                              GL_DYNAMIC_STORAGE_BIT);
    
    ogl->glNamedBufferStorage(ogl->buffers[Opengl__VBO_Colors], 
                              sizeof(Vec4f) * OPENGL_MAX_ENTITIES, 
                              nullptr, 
                              GL_DYNAMIC_STORAGE_BIT);
    
    ogl->glNamedBufferStorage(ogl->buffers[Opengl__VBO_Transform], 
                              sizeof(Mat4f) * OPENGL_MAX_ENTITIES, 
                              nullptr, 
                              GL_DYNAMIC_STORAGE_BIT);
    
    
    // NOTE(Momo): Setup VAO
    ogl->glCreateVertexArrays(1, &ogl->model);
    ogl->glVertexArrayVertexBuffer(ogl->model, 
                                   Opengl__VAO_Binding_Model, 
                                   ogl->buffers[Opengl__VBO_Model], 
                                   0, 
                                   sizeof(f32)*3);
    
    ogl->glVertexArrayVertexBuffer(ogl->model, 
                                   Opengl__VAO_Binding_Texture, 
                                   ogl->buffers[Opengl__VBO_Texture], 
                                   0, 
                                   sizeof(f32) * 8);
    
    ogl->glVertexArrayVertexBuffer(ogl->model, 
                                   Opengl__VAO_Binding_Colors, 
                                   ogl->buffers[Opengl__VBO_Colors],  
                                   0, 
                                   sizeof(Vec4f));
    
    ogl->glVertexArrayVertexBuffer(ogl->model, 
                                   Opengl__VAO_Binding_Transform, 
                                   ogl->buffers[Opengl__VBO_Transform], 
                                   0, 
                                   sizeof(Mat4f));
    
    // NOTE(Momo): Setup Attributes
    // aModelVtx
    ogl->glEnableVertexArrayAttrib(ogl->model, Opengl__ATB_Model); 
    ogl->glVertexArrayAttribFormat(ogl->model, 
                                   Opengl__ATB_Model, 
                                   3, 
                                   GL_FLOAT, 
                                   GL_FALSE, 
                                   0);
    
    ogl->glVertexArrayAttribBinding(ogl->model, 
                                    Opengl__ATB_Model, 
                                    Opengl__VAO_Binding_Model);
    
    // aColor
    ogl->glEnableVertexArrayAttrib(ogl->model, Opengl__ATB_Colors); 
    ogl->glVertexArrayAttribFormat(ogl->model, 
                                   Opengl__ATB_Colors, 
                                   4, 
                                   GL_FLOAT, GL_FALSE, 0);
    ogl->glVertexArrayAttribBinding(ogl->model, 
                                    Opengl__ATB_Colors, 
                                    Opengl__VAO_Binding_Colors);
    
    ogl->glVertexArrayBindingDivisor(ogl->model, Opengl__VAO_Binding_Colors, 1); 
    
    // aTexCoord
    ogl->glEnableVertexArrayAttrib(ogl->model, Opengl__ATB_Texture_1); 
    ogl->glVertexArrayAttribFormat(ogl->model, 
                                   Opengl__ATB_Texture_1, 
                                   2, 
                                   GL_FLOAT, 
                                   GL_FALSE,
                                   sizeof(Vec2f) * 0);
    
    ogl->glEnableVertexArrayAttrib(ogl->model, Opengl__ATB_Texture_2); 
    ogl->glVertexArrayAttribFormat(ogl->model, 
                                   Opengl__ATB_Texture_2, 
                                   2, 
                                   GL_FLOAT, 
                                   GL_FALSE, 
                                   sizeof(Vec2f) * 1);
    
    ogl->glEnableVertexArrayAttrib(ogl->model, Opengl__ATB_Texture_3); 
    ogl->glVertexArrayAttribFormat(ogl->model, 
                                   Opengl__ATB_Texture_3, 
                                   2, 
                                   GL_FLOAT, 
                                   GL_FALSE, 
                                   sizeof(Vec2f) * 2);
    
    ogl->glEnableVertexArrayAttrib(ogl->model, Opengl__ATB_Texture_4); 
    ogl->glVertexArrayAttribFormat(ogl->model, 
                                   Opengl__ATB_Texture_4, 
                                   2, 
                                   GL_FLOAT, 
                                   GL_FALSE, 
                                   sizeof(Vec2f) * 3);
    
    ogl->glVertexArrayAttribBinding(ogl->model, 
                                    Opengl__ATB_Texture_1, 
                                    Opengl__VAO_Binding_Texture);
    
    ogl->glVertexArrayAttribBinding(ogl->model, 
                                    Opengl__ATB_Texture_2, 
                                    Opengl__VAO_Binding_Texture);
    
    ogl->glVertexArrayAttribBinding(ogl->model, 
                                    Opengl__ATB_Texture_3, 
                                    Opengl__VAO_Binding_Texture);
    
    ogl->glVertexArrayAttribBinding(ogl->model, 
                                    Opengl__ATB_Texture_4, 
                                    Opengl__VAO_Binding_Texture);
    
    ogl->glVertexArrayBindingDivisor(ogl->model, 
                                     Opengl__VAO_Binding_Texture, 
                                     1); 
    
    
    // aTransform
    ogl->glEnableVertexArrayAttrib(ogl->model, Opengl__ATB_Transform_1); 
    ogl->glVertexArrayAttribFormat(ogl->model, 
                                   Opengl__ATB_Transform_1, 
                                   4, 
                                   GL_FLOAT, 
                                   GL_FALSE, 
                                   sizeof(f32) * 0 * 4);
    ogl->glEnableVertexArrayAttrib(ogl->model, Opengl__ATB_Transform_2);
    ogl->glVertexArrayAttribFormat(ogl->model, 
                                   Opengl__ATB_Transform_2, 
                                   4, 
                                   GL_FLOAT, 
                                   GL_FALSE, 
                                   sizeof(f32) * 1 * 4);
    ogl->glEnableVertexArrayAttrib(ogl->model, Opengl__ATB_Transform_3); 
    ogl->glVertexArrayAttribFormat(ogl->model, 
                                   Opengl__ATB_Transform_3, 
                                   4, 
                                   GL_FLOAT, 
                                   GL_FALSE, 
                                   sizeof(f32) * 2 * 4);
    ogl->glEnableVertexArrayAttrib(ogl->model, Opengl__ATB_Transform_4); 
    ogl->glVertexArrayAttribFormat(ogl->model, 
                                   Opengl__ATB_Transform_4,
                                   4, 
                                   GL_FLOAT, 
                                   GL_FALSE, 
                                   sizeof(f32) * 3 * 4);
    
    ogl->glVertexArrayAttribBinding(ogl->model, 
                                    Opengl__ATB_Transform_1, 
                                    Opengl__VAO_Binding_Transform);
    ogl->glVertexArrayAttribBinding(ogl->model, 
                                    Opengl__ATB_Transform_2, 
                                    Opengl__VAO_Binding_Transform);
    ogl->glVertexArrayAttribBinding(ogl->model, 
                                    Opengl__ATB_Transform_3, 
                                    Opengl__VAO_Binding_Transform);
    ogl->glVertexArrayAttribBinding(ogl->model, 
                                    Opengl__ATB_Transform_4, 
                                    Opengl__VAO_Binding_Transform);
    
    ogl->glVertexArrayBindingDivisor(ogl->model, 
                                     Opengl__VAO_Binding_Transform, 
                                     1); 
    
    // NOTE(Momo): alpha blend
    ogl->glEnable(GL_BLEND);
    ogl->glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    // NOTE(Momo): Setup indices
    ogl->glVertexArrayElementBuffer(ogl->model, 
                                    ogl->buffers[Opengl__VBO_Indices]);
    
    
    
    // NOTE(Momo): Setup shader Program
    ogl->shader = ogl->glCreateProgram();
    Opengl__AttachShader(ogl,
                         ogl->shader, 
                         GL_VERTEX_SHADER, 
                         (char*)ogl->vertex_shader);
    Opengl__AttachShader(ogl,
                         ogl->shader, 
                         GL_FRAGMENT_SHADER, 
                         (char*)ogl->fragment_shader);
    
    ogl->glLinkProgram(ogl->shader);
    
    GLint Result;
    ogl->glGetProgramiv(ogl->shader, GL_LINK_STATUS, &Result);
    if (Result != GL_TRUE) {
        char msg[KIBIBYTE];
        ogl->glGetProgramInfoLog(ogl->shader, KIBIBYTE, nullptr, msg);
		return false;
    }
    Opengl__AddPredefinedTextures(ogl);
	
	return true;
    
}


void 
Opengl__DrawInstances(Opengl* ogl,
                      GLuint texture, 
                      u32 instances_to_draw, 
                      u32 index_to_draw_from) 
{
    Assert(instances_to_draw + index_to_draw_from < OPENGL_MAX_ENTITIES);
    if (instances_to_draw > 0) {
        ogl->glBindTexture(GL_TEXTURE_2D, texture);
        ogl->glTexParameteri(GL_TEXTURE_2D, 
                             GL_TEXTURE_MIN_FILTER, 
                             GL_NEAREST);
        ogl->glTexParameteri(GL_TEXTURE_2D, 
                             GL_TEXTURE_MAG_FILTER, 
                             GL_NEAREST);
        ogl->glEnable(GL_BLEND);
        ogl->glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        ogl->glBindVertexArray(ogl->model);
        ogl->glUseProgram(ogl->shader);
        
        ogl->glDrawElementsInstancedBaseInstance(GL_TRIANGLES, 
                                                 6, 
                                                 GL_UNSIGNED_BYTE, 
                                                 nullptr, 
                                                 instances_to_draw,
                                                 index_to_draw_from);
    }
}


Renderer_Texture_Handle
Opengl_AddTexture(Opengl* ogl,
                  u32 width,
                  u32 height,
                  void* pixels) 
{
    Renderer_Texture_Handle ret = {};
    
    if (List_Remaining(&ogl->textures) == 0) {
        ret.success = false;
        ret.id = 0;
        return ret;
    }
    
    
    GLuint entry;
    
    ogl->glCreateTextures(GL_TEXTURE_2D, 
                          1, 
                          &entry);
    
    ogl->glTextureStorage2D(entry, 
                            1, 
                            GL_RGBA8, 
                            width, 
                            height);
    
    ogl->glTextureSubImage2D(entry, 
                             0, 
                             0, 
                             0, 
                             width, 
                             height, 
                             GL_RGBA, 
                             GL_UNSIGNED_BYTE, 
                             pixels);
    
    ret.id = (u32)ogl->textures.count;
    ret.success = true;
    List_PushItem(&ogl->textures, entry);
    return ret;
}

static inline void
Opengl_ClearTextures(Opengl* ogl) {
    ogl->glDeleteTextures((s32)ogl->textures.count, 
                          ogl->textures.data);
    List_Clear(&ogl->textures);
    Opengl__AddPredefinedTextures(ogl);
}

static inline Mailbox*
Opengl_BeginFrame(Opengl* ogl, Vec2u render_wh, Rect2u region) {
	Opengl__AlignViewport(ogl, render_wh, region);
	Mailbox_Clear(&ogl->render_commands);
	return &ogl->render_commands;
}

static inline void
Opengl_EndFrame(Opengl* ogl, Mailbox* commands) 
{
    GLuint current_texture = 0;
    u32 instances_to_draw = 0;
    u32 last_drawn_instance_index = 0;
    u32 current_instance_index = 0;
    
	
    
    for (u32 i = 0; i < commands->entry_count; ++i) {
        Mailbox_EntryHeader* entry = Mailbox_GetEntry(commands, i);
        
        switch(entry->type) {
            
            case Renderer_CommandType_SetBasis: {
                auto* data = (Renderer_Command_SetBasis*)
                    Mailbox_GetEntryData(commands, entry);
                
                Opengl__DrawInstances(ogl,
                                      current_texture, 
                                      instances_to_draw, 
                                      last_drawn_instance_index);
                last_drawn_instance_index += instances_to_draw;
                instances_to_draw = 0;
                
                auto result = Mat_Transpose(data->basis);
                GLint uProjectionLoc = ogl->glGetUniformLocation(ogl->shader,
                                                                 "uProjection");
                
                ogl->glProgramUniformMatrix4fv(ogl->shader, 
                                               uProjectionLoc, 
                                               1, 
                                               GL_FALSE, 
                                               (const GLfloat*)&result);
            } break;
            case Renderer_CommandType_ClearColor: {
                auto* data = (Renderer_Command_ClearColor*)
                    Mailbox_GetEntryData(commands, entry);
				
                ogl->glClearColor(data->colors.r, 
                                  data->colors.g, 
                                  data->colors.b, 
                                  data->colors.a);
				ogl->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
				
            } break;
            case Renderer_CommandType_DrawQuad: {
                auto* data = (Renderer_Command_DrawQuad*)
                    Mailbox_GetEntryData(commands, entry);
                
                GLuint ogl_texture_handle = *(ogl->textures + Opengl__Predefined_Texture_ID_Blank);
                
                // NOTE(Momo): If the currently set texture is not same as the 
                // currently processed texture, batch draw all instances before 
                // the current instance.
                if (current_texture != ogl_texture_handle) {
                    Opengl__DrawInstances(ogl,
                                          current_texture, 
                                          instances_to_draw, 
                                          last_drawn_instance_index);
                    last_drawn_instance_index += instances_to_draw;
                    instances_to_draw = 0;
                    current_texture = ogl_texture_handle;
                }
                
                // NOTE(Momo): Update the current instance values
                ogl->glNamedBufferSubData(ogl->buffers[Opengl__VBO_Colors], 
                                          current_instance_index * sizeof(Vec4f),
                                          sizeof(Vec4f), 
                                          &data->colors);
                
                ogl->glNamedBufferSubData(ogl->buffers[Opengl__VBO_Texture],
                                          current_instance_index * sizeof(ogl->quad_uv),
                                          sizeof(ogl->quad_uv),
                                          &ogl->quad_uv);
                
                // NOTE(Momo): Transpose; game is row-major
                Mat4f transform = Mat_Transpose(data->transform);
                ogl->glNamedBufferSubData(ogl->buffers[Opengl__VBO_Transform], 
                                          current_instance_index* sizeof(Mat4f), 
                                          sizeof(Mat4f), 
                                          &transform);
                
                // NOTE(Momo): Update Bookkeeping
                ++instances_to_draw;
                ++current_instance_index;
            } break;
            case Renderer_CommandType_DrawTexturedQuad: {
                auto* data = (Renderer_Command_DrawTexturedQuad*)
                    Mailbox_GetEntryData(commands, entry);
                
                GLuint texture_handle = *(ogl->textures + data->texture_handle.id); 
                
                // NOTE(Momo): If the currently set texture is not same as the currently
                // processed texture, batch draw all instances before the current instance.
                if (current_texture != texture_handle) {
                    Opengl__DrawInstances(ogl,
                                          current_texture, 
                                          instances_to_draw, 
                                          last_drawn_instance_index);
                    last_drawn_instance_index += instances_to_draw;
                    instances_to_draw = 0;
                    current_texture = texture_handle;
                }
                
                // NOTE(Momo): Update the current instance values
                ogl->glNamedBufferSubData(ogl->buffers[Opengl__VBO_Colors], 
                                          current_instance_index * sizeof(Vec4f),
                                          sizeof(Vec4f), 
                                          &data->colors);
                
                f32 texture_uv_rect_in_vertices[] = {
                    data->texture_uv_rect.min.x, data->texture_uv_rect.max.y,
                    data->texture_uv_rect.max.x, data->texture_uv_rect.max.y,
                    data->texture_uv_rect.max.x, data->texture_uv_rect.min.y,
                    data->texture_uv_rect.min.x, data->texture_uv_rect.min.y
                };
                ogl->glNamedBufferSubData(ogl->buffers[Opengl__VBO_Texture],
                                          current_instance_index * sizeof(ogl->quad_uv),
                                          sizeof(ogl->quad_uv),
                                          &texture_uv_rect_in_vertices);
                
                // NOTE(Momo): Transpose; game is row-major
                Mat4f transform = Mat_Transpose(data->transform);
                ogl->glNamedBufferSubData(ogl->buffers[Opengl__VBO_Transform], 
                                          current_instance_index* sizeof(Mat4f), 
                                          sizeof(Mat4f), 
                                          &transform);
                
                // NOTE(Momo): Update Bookkeeping
                ++instances_to_draw;
                ++current_instance_index;
                
            } break;
        }
    }
    
    Opengl__DrawInstances(ogl, current_texture, instances_to_draw, last_drawn_instance_index);
    
}



#endif

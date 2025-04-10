/*!
 * @file
 * @brief This file contains implementation of gpu
 *
 * @author Tomáš Milet, imilet@fit.vutbr.cz
 */

#include <student/gpu.hpp>

#define MIN(a,b) (a < b ? (a) : (b))
#define MAX(a,b) (a > b ? (a) : (b))

struct Primitive{
  OutVertex vertex[3];
};

struct Barycentric{
  float area;
  float lambda0;
  float lambda1;
  float lambda2;
};

void indexing(GPUMemory& mem, uint32_t i, InVertex& inVertex){
  int32_t bufferID =  mem.vertexArrays[mem.activatedVertexArray].indexBufferID;
  IndexType type = mem.vertexArrays[mem.activatedVertexArray].indexType;
  uint64_t offset = mem.vertexArrays[mem.activatedVertexArray].indexOffset;

  // Indexing
  if(bufferID == -1)
    inVertex.gl_VertexID = i;
  else {
    if(type == IndexType::UINT8){
      uint8_t* index = (uint8_t*)((uint8_t*) mem.buffers[bufferID].data + offset);
      inVertex.gl_VertexID = (uint8_t) index[i];
    }

    if(type == IndexType::UINT16){
      uint16_t* index = (uint16_t*)((uint8_t*) mem.buffers[bufferID].data + offset);
      inVertex.gl_VertexID = (uint16_t) index[i];
    }

    if(type == IndexType::UINT32){
      uint32_t* index = (uint32_t*)((uint8_t*) mem.buffers[bufferID].data + offset);
      inVertex.gl_VertexID = (uint32_t) index[i];
    }
  }
}

void vertex_attributes(GPUMemory& mem,InVertex& inVertex){
      for (uint32_t a = 0; a < maxAttributes; ++a){
      
      VertexAttrib va = mem.vertexArrays[mem.activatedVertexArray].vertexAttrib[a];

      if(va.type != AttributeType::EMPTY){
        Attribute *attribute = (Attribute*) ((uint8_t*) mem.buffers[va.bufferID].data + va.offset + va.stride*inVertex.gl_VertexID);

        if(va.type == AttributeType::FLOAT)
          inVertex.attributes[a].v1 = *((float*)attribute);

        if(va.type == AttributeType::VEC2)
          inVertex.attributes[a].v2 = *((glm::vec2*)attribute);

        if(va.type == AttributeType::VEC3)
          inVertex.attributes[a].v3 = *((glm::vec3*)attribute);

        if(va.type == AttributeType::VEC4)
          inVertex.attributes[a].v4 = *((glm::vec4*)attribute);
      }
    }
}

void prespective_division(Primitive& primitive) {
  // Deleni slozek [x,y,z] vektoru slozkou w
  for (uint32_t i = 0; i < 3; ++i){
    primitive.vertex[i].gl_Position.x /= primitive.vertex[i].gl_Position.w;
    primitive.vertex[i].gl_Position.y /= primitive.vertex[i].gl_Position.w;
    primitive.vertex[i].gl_Position.z /= primitive.vertex[i].gl_Position.w;
  }
}

void viewport_transformation(Framebuffer& fb, Primitive& primitive) {
  // Rozliseni obrazovky
  uint32_t width = fb.width, height = fb.height;

  // (x*0.5+0.5)*w, (y*0.5+0.5)*h
  for (uint32_t i = 0; i < 3; ++i){
    primitive.vertex[i].gl_Position.x = (primitive.vertex[i].gl_Position.x*.5+.5)*width;
    primitive.vertex[i].gl_Position.y = (primitive.vertex[i].gl_Position.y*.5+.5)*height;
  }
}

void fragment_attributes(Primitive& primitive, Barycentric& barycentrics, InFragment& inFragment, Program& prg){
  float h0 = primitive.vertex[0].gl_Position.w,
        h1 = primitive.vertex[1].gl_Position.w,
        h2 = primitive.vertex[2].gl_Position.w,
        s;

  s = (barycentrics.lambda0/h0) + (barycentrics.lambda1/h1) + (barycentrics.lambda2/h2);

  float pLambda0 = barycentrics.lambda0 / (h0 * s),
        pLambda1 = barycentrics.lambda1 / (h1 * s),
        pLambda2 = barycentrics.lambda2 / (h2 * s);

  for (uint32_t a = 0; a < maxAttributes; ++a){
      
  Attribute a0 = primitive.vertex[0].attributes[a],
            a1 = primitive.vertex[1].attributes[a],
            a2 = primitive.vertex[2].attributes[a];

  AttributeType type = prg.vs2fs[a];

  if(type != AttributeType::EMPTY){

    if(type == AttributeType::FLOAT)
          inFragment.attributes[a].v1 = a0.v1*pLambda0 + a1.v1*pLambda1 + a2.v1*pLambda2;

    if(type == AttributeType::VEC2)
          inFragment.attributes[a].v2 = a0.v2*pLambda0 + a1.v2*pLambda1 + a2.v2*pLambda2;

    if(type == AttributeType::VEC3)
          inFragment.attributes[a].v3 = a0.v3*pLambda0 + a1.v3*pLambda1 + a2.v3*pLambda2;

    if(type == AttributeType::VEC4)
          inFragment.attributes[a].v4 = a0.v4*pLambda0 + a1.v4*pLambda1 + a2.v4*pLambda2;
    }
  }
}

void perFragmentOperations(Framebuffer& fb, OutFragment outFragment, float inFragmentZcoord, uint32_t x, uint32_t y){
  if(outFragment.discard == false){
    float* depth = nullptr;
    uint8_t* pixel = nullptr;

    if(fb.depth.data == nullptr || fb.color.data == nullptr)
      return;

    if(fb.yReversed == false){
      // Efektivni adresa hloubky a barvy ve framebufferu
      depth = (float*) getPixel(fb.depth, x, y);
      pixel = (uint8_t*) getPixel(fb.color, x, y);
    } else {
      depth = (float*) getPixel(fb.depth, x, fb.height - y - 1);
      pixel = (uint8_t*) getPixel(fb.color, x, fb.height - y - 1);
    }

    //a - kanal nepruhlednosti
    float a = outFragment.gl_FragColor[3];

    if(inFragmentZcoord < *depth) {
      *depth = inFragmentZcoord;

      if(a == 1.0f){
        for (uint32_t i = 0; i < fb.color.channels; ++i)
          pixel[fb.color.channelTypes[i]] = (uint8_t) (outFragment.gl_FragColor[i] * 255);
      } else {
        glm::vec4 framebuffer_color(pixel[0] / 255.f, pixel[1] / 255.f,  pixel[2] / 255.f, a);
        glm::vec4 fragcolor = outFragment.gl_FragColor;

        glm::vec3 final_color = fragcolor*a + framebuffer_color*(1.0f-a);

        for (uint32_t i = 0; i < fb.color.channels - 1; ++i)
        pixel[fb.color.channelTypes[i]] = (uint8_t) (std::round((final_color[i] * 255)*100.f)/100.f);
      }
    }
  }
}

void rasterize(GPUMemory& mem, Framebuffer& fb, Primitive& primitive, bool backfaceCulling, Program prg) {
  // Barycentricke souradnice + obsah trojuhelniku
  Barycentric barycentrics;

  // Hranice trojuhelniku: V0-V1, V1-V2, V2-V0
  glm::vec3 primitive_edge[3];

  /// Vypocet hran trojuhelniku
  for (uint32_t i = 0; i < 3; ++i)
      primitive_edge[i] = primitive.vertex[(i+1)%3].gl_Position - primitive.vertex[i].gl_Position;

  /// Obsah trojuhelniku (vznikne nasobkem 2 hran, je ve slozce: z) --> 0.5 * (vektor1 x vektor2) 
  barycentrics.area = (glm::cross(primitive_edge[0],primitive_edge[1]) * 0.5f).z;;

  /// Kontrola zda jsou hrany trojuhelniku clockwise
  bool cw = (glm::cross(primitive_edge[0], primitive_edge[1]).z < 0 || glm::cross(primitive_edge[1], primitive_edge[2]).z < 0 || glm::cross(primitive_edge[2], primitive_edge[0]).z < 0);

  // backface culling a trojuhelnik je clock wise, vykresleni se neprovede
  if(backfaceCulling && cw)
        return;

  // boundary box vykreslovaneho primitiva - optimalizace
  int x_MIN = MAX(0, MIN(primitive.vertex[0].gl_Position.x, MIN(primitive.vertex[1].gl_Position.x,primitive.vertex[2].gl_Position.x))),
      y_MIN = MAX(0, MIN(primitive.vertex[0].gl_Position.y, MIN(primitive.vertex[1].gl_Position.y,primitive.vertex[2].gl_Position.y))),
      x_MAX = MIN(fb.width, MAX(primitive.vertex[0].gl_Position.x, MAX(primitive.vertex[1].gl_Position.x,primitive.vertex[2].gl_Position.x))),
      y_MAX = MIN(fb.height, MAX(primitive.vertex[0].gl_Position.y, MAX(primitive.vertex[1].gl_Position.y,primitive.vertex[2].gl_Position.y)));

  // Zmena souradnic [x,y] width * height
  for (int y = y_MIN; y < y_MAX; ++y)
  {
    for (int x = x_MIN; x < x_MAX; ++x)
    {
      // Stred aktualniho pixelu
      glm::vec3 pixelCoord((float) x + 0.5f, (float) y + 0.5f, 0.f);

      // Vypocet barycentrickych souradnic bodu [x,y]
      barycentrics.lambda0 = ((glm::cross(primitive_edge[1], pixelCoord - glm::vec3(primitive.vertex[1].gl_Position)).z)*0.5f) / barycentrics.area;
      barycentrics.lambda1 = ((glm::cross(primitive_edge[2], pixelCoord - glm::vec3(primitive.vertex[2].gl_Position)).z)*0.5f) / barycentrics.area;
      barycentrics.lambda2 = ((glm::cross(primitive_edge[0], pixelCoord - glm::vec3(primitive.vertex[0].gl_Position)).z)*0.5f) / barycentrics.area;

      float lambda_sum = (barycentrics.lambda0 + barycentrics.lambda1 + barycentrics.lambda2);

      if(((lambda_sum <= 1.0f + 1e-2) && barycentrics.lambda0 >= 0.0f && barycentrics.lambda1 >= 0.0f && barycentrics.lambda2 >= 0.0f)){
        InFragment inFragment;
        
        // Hloubka fragmentu
        inFragment.gl_FragCoord.x = pixelCoord.x;
        inFragment.gl_FragCoord.y = pixelCoord.y;
        inFragment.gl_FragCoord.z = barycentrics.lambda0 * primitive.vertex[0].gl_Position.z + barycentrics.lambda1 * primitive.vertex[1].gl_Position.z + barycentrics.lambda2 * primitive.vertex[2].gl_Position.z;

        // Interpolace atributu fragmentu
        fragment_attributes(primitive, barycentrics, inFragment, prg);

        OutFragment outFragment;
        
        ShaderInterface si;
        si.uniforms = mem.uniforms;
        si.textures = mem.textures;
        si.gl_DrawID = mem.gl_DrawID;

        /// Fragment shader
        prg.fragmentShader(outFragment, inFragment, si);
        
        /// PerFragmentOperace
        // Orezani barvy do intervalu <0,1> 
        glm::clamp(outFragment.gl_FragColor, 0.f, 1.f);
        perFragmentOperations(fb, outFragment, inFragment.gl_FragCoord.z, x, y);
      }
    }
  }
}

void draw(GPUMemory& mem, DrawCommand cmd){
  Program prg = mem.programs[mem.activatedProgram];

  Primitive primitive;
  
  for (uint32_t i = 0; i < cmd.nofVertices; ++i)
  {
    InVertex inVertex;
    OutVertex outVertex;
    ShaderInterface si;

    /// Vertex Assembly - sestaveni vrcholu
    // Indexing, i = invokace vertex shaderu
    indexing(mem, i, inVertex);
    // Nacteni atributu z bufferu
    vertex_attributes(mem, inVertex);

    /// Shader interface - rozhrani shaderu
    si.gl_DrawID = mem.gl_DrawID; 
    si.uniforms = mem.uniforms;
    si.textures = mem.textures; 

    /// Vertex Shader --> outVertex
    prg.vertexShader(outVertex, inVertex, si);

    primitive.vertex[i % 3] = outVertex;

    /// Mame-li 3 vrcholy --> provede se perspektivni deleni, viewport transformace, rasterizace, ...
    if((i+1) % 3 == 0) {
      prespective_division(primitive);
      viewport_transformation(mem.framebuffers[mem.activatedFramebuffer], primitive);
      rasterize(mem, mem.framebuffers[mem.activatedFramebuffer], primitive, cmd.backfaceCulling, prg);
    }
  }

  //Inkrementace DrawID po kazdem prikazu vykreslovani
  ++mem.gl_DrawID;
}

/**
 * @brief This function clears the framebuffer (helper function)
 * 
 * @param GPU memory
 * @param Clear Command
 */
void clear(GPUMemory&mem, ClearCommand cmd){
  // Ukazatel na aktivovany framebuffer (zacatek + posun)
  Framebuffer *fbp = mem.framebuffers + mem.activatedFramebuffer;

  // Cisteni barvy framebufferu za predpokladu, ze data nejsou nullptr
  if(cmd.clearColor && fbp->color.data != nullptr){ 
    // Rozliseni obrazovky x*y ([x,y]) 
    for (uint32_t y = 0; y < fbp->height; ++y){
      for (uint32_t x = 0; x < fbp->width; ++x){
        // Efektivni adresa
        uint8_t* pixel = ((uint8_t*)fbp->color.data) + y*fbp->color.pitch + x*fbp->color.bytesPerPixel;
        // Vycisteni barev
        for (uint32_t i = 0; i < fbp->color.channels; ++i)
          pixel[fbp->color.channelTypes[i]] = (uint8_t) (cmd.color[i] * 255);
      }
    }
  }

  // Cisteni hloubky framebufferu za predpokladu, ze data nejsou nullptr
  if(cmd.clearDepth && fbp->depth.data != nullptr){
    // Rozliseni obrazovky x*y ([x,y])
    for (uint32_t y = 0; y < fbp->height; ++y){
      for (uint32_t x = 0; x < fbp->width; ++x){
        // Efektivni adresa
        float* pixel = (float*) ((uint8_t*)fbp->depth.data + y*fbp->depth.pitch + x*fbp->depth.bytesPerPixel);
        // Vycisteni hloubky
        *pixel = cmd.depth;
      }
    }
  }
}

void subcommand(GPUMemory&mem, SubCommand cmd) {
  CommandBuffer cb = *(cmd.commandBuffer);

    for(uint32_t i = 0; i < cb.nofCommands; ++i){
      CommandType type = cb.commands[i].type;
      CommandData data = cb.commands[i].data;

      if(type == CommandType::CLEAR)
        clear(mem, data.clearCommand);

      if(type == CommandType::SET_DRAW_ID)
        mem.gl_DrawID = data.setDrawIdCommand.id;

      if(type == CommandType::DRAW)
        draw(mem, data.drawCommand);      

      if(type == CommandType::BIND_FRAMEBUFFER)
        mem.activatedFramebuffer = data.bindFramebufferCommand.id;

      if(type == CommandType::BIND_PROGRAM)
        mem.activatedProgram = data.bindProgramCommand.id;

      if(type == CommandType::BIND_VERTEXARRAY)
        mem.activatedVertexArray = data.bindVertexArrayCommand.id;

      if(type == CommandType::SUB_COMMAND)
        subcommand(mem, data.subCommand);

    }
}

//! [izg_enqueue]
void izg_enqueue(GPUMemory&mem,CommandBuffer const&cb){
  (void)mem;
  (void)cb;
  /// \todo Tato funkce reprezentuje funkcionalitu grafické karty.<br>
  /// Měla by umět zpracovat command buffer, čistit framebuffer a kreslit.<br>
  /// mem obsahuje paměť grafické karty.
  /// cb obsahuje command buffer pro zpracování.
  /// Bližší informace jsou uvedeny na hlavní stránce dokumentace.

  //Vynulovani pri kazdem volani funkce enqueue
  mem.gl_DrawID = 0;

  for(uint32_t i = 0; i < cb.nofCommands; ++i){
      CommandType type = cb.commands[i].type;
      CommandData data = cb.commands[i].data;
      
      if(type == CommandType::CLEAR){
        clear(mem, data.clearCommand);
      }

      if(type == CommandType::SET_DRAW_ID){
        mem.gl_DrawID = data.setDrawIdCommand.id;
      }

      if(type == CommandType::DRAW){
        draw(mem, data.drawCommand);      
      }

      if(type == CommandType::BIND_FRAMEBUFFER){
        mem.activatedFramebuffer = data.bindFramebufferCommand.id;
      }

      if(type == CommandType::BIND_PROGRAM){
        mem.activatedProgram = data.bindProgramCommand.id;
      }

      if(type == CommandType::BIND_VERTEXARRAY){
        mem.activatedVertexArray = data.bindVertexArrayCommand.id;
      }

      if(type == CommandType::SUB_COMMAND){
        subcommand(mem, data.subCommand);
      }
    }
}
//! [izg_enqueue]

/**
 * @brief This function reads color from texture.
 *
 * @param texture texture
 * @param uv uv coordinates
 *
 * @return color 4 floats
 */
glm::vec4 read_texture(Texture const&texture,glm::vec2 uv){
  if(!texture.img.data)return glm::vec4(0.f);
  auto&img = texture.img;
  auto uv1 = glm::fract(glm::fract(uv)+1.f);
  auto uv2 = uv1*glm::vec2(texture.width-1,texture.height-1)+0.5f;
  auto pix = glm::uvec2(uv2);
  return texelFetch(texture,pix);
}

/**
 * @brief This function reads color from texture with clamping on the borders.
 *
 * @param texture texture
 * @param uv uv coordinates
 *
 * @return color 4 floats
 */
glm::vec4 read_textureClamp(Texture const&texture,glm::vec2 uv){
  if(!texture.img.data)return glm::vec4(0.f);
  auto&img = texture.img;
  auto uv1 = glm::clamp(uv,0.f,1.f);
  auto uv2 = uv1*glm::vec2(texture.width-1,texture.height-1)+0.5f;
  auto pix = glm::uvec2(uv2);
  return texelFetch(texture,pix);
}

/**
 * @brief This function fetches color from texture.
 *
 * @param texture texture
 * @param pix integer coorinates
 *
 * @return color 4 floats
 */
glm::vec4 texelFetch(Texture const&texture,glm::uvec2 pix){
  auto&img = texture.img;
  glm::vec4 color = glm::vec4(0.f,0.f,0.f,1.f);
  if(pix.x>=texture.width || pix.y >=texture.height)return color;
  if(img.format == Image::UINT8){
    auto colorPtr = (uint8_t*)getPixel(img,pix.x,pix.y);
    for(uint32_t c=0;c<img.channels;++c)
      color[c] = colorPtr[img.channelTypes[c]]/255.f;
  }
  if(texture.img.format == Image::FLOAT32){
    auto colorPtr = (float*)getPixel(img,pix.x,pix.y);
    for(uint32_t c=0;c<img.channels;++c)
      color[c] = colorPtr[img.channelTypes[c]];
  }
  return color;
}


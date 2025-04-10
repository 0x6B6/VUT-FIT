/*!
 * @file
 * @brief This file contains functions for model rendering
 *
 * @author Tomáš Milet, imilet@fit.vutbr.cz
 */
#include <student/prepareModel.hpp>
#include <student/gpu.hpp>

///\endcond
void prepareNode(GPUMemory&mem,CommandBuffer&cb,Node const&node,Model const&model,glm::mat4 const&matrix, uint32_t& drawCounter){
  if(node.mesh > -1) {
    Mesh mesh = model.meshes[node.mesh];

    VertexArray va = {
      .indexBufferID = mesh.indexBufferID,
      .indexOffset = mesh.indexOffset,
      .indexType = mesh.indexType
    };

    va.vertexAttrib[0] = mesh.position;
    va.vertexAttrib[1] = mesh.normal;
    va.vertexAttrib[2] = mesh.texCoord;

    mem.vertexArrays[drawCounter] = va;

    /// Command BufferS
    BindVertexArrayCommand bindVaCmd = {.id = drawCounter};
    cb.commands[cb.nofCommands].data.bindVertexArrayCommand = bindVaCmd;
    cb.commands[cb.nofCommands++].type = CommandType::BIND_VERTEXARRAY;

    DrawCommand drawCmd;
    drawCmd.backfaceCulling = mesh.doubleSided ? false : true;
    drawCmd.nofVertices = mesh.nofIndices;
    cb.commands[cb.nofCommands].data.drawCommand = drawCmd;
    cb.commands[cb.nofCommands++].type = CommandType::DRAW;

    /// Prubezna matice, inverzni-transponovana matice
    glm::mat4 modelMatrix = matrix * node.modelMatrix;
    glm::mat4 inverseModelMatrix = glm::transpose(glm::inverse(modelMatrix));

    mem.uniforms[10+(drawCounter)*5+0].m4 = modelMatrix;
    mem.uniforms[10+(drawCounter)*5+1].m4 = inverseModelMatrix;
    mem.uniforms[10+(drawCounter)*5+2].v4 = mesh.diffuseColor;
    mem.uniforms[10+(drawCounter)*5+3].i1 = mesh.diffuseTexture;
    mem.uniforms[10+(drawCounter)*5+4].v1 = mesh.doubleSided;

    ++drawCounter;
  }
  
  for(size_t i = 0; i < node.children.size(); ++i)
    prepareNode(mem, cb, node.children[i], model, matrix * node.modelMatrix, drawCounter);
}

/**
 * @brief This function prepares model into memory and creates command buffer
 *
 * @param mem gpu memory
 * @param commandBuffer command buffer
 * @param model model structure
 */
//! [drawModel]
void prepareModel(GPUMemory&mem,CommandBuffer&commandBuffer,Model const&model){
  (void)mem;
  (void)commandBuffer;
  (void)model;
  /// \todo Tato funkce připraví command buffer pro model a nastaví správně pamět grafické karty.<br>
  /// Vaším úkolem je správně projít model a vložit vykreslovací příkazy do commandBufferu.
  /// Zároveň musíte vložit do paměti textury, buffery a uniformní proměnné, které buffer command buffer využívat.
  /// Bližší informace jsou uvedeny na hlavní stránce dokumentace a v testech.

  /// Nastaveni pameti gpu
  for (uint32_t i = 0; i < model.buffers.size(); ++i)
    mem.buffers[i] = model.buffers[i];

   for (uint32_t i = 0; i < model.textures.size(); ++i)
    mem.textures[i] = model.textures[i];   

  glm::mat4 matrix = glm::mat4(1.f);

  uint32_t drawCounter = 0;

  for (size_t i = 0; i < model.roots.size(); ++i)
    prepareNode(mem, commandBuffer, model.roots[i], model, matrix, drawCounter);
}
//! [drawModel]

/**
 * @brief This function represents vertex shader of texture rendering method.
 *
 * @param outVertex output vertex
 * @param inVertex input vertex
 * @param si shader interface
 */
//! [drawModel_vs]
void drawModel_vertexShader(OutVertex&outVertex,InVertex const&inVertex,ShaderInterface const&si){
  (void)outVertex;
  (void)inVertex;
  (void)si;
  /// \todo Tato funkce reprezentujte vertex shader.<br>
  /// Vaším úkolem je správně trasnformovat vrcholy modelu.
  /// Bližší informace jsou uvedeny na hlavní stránce dokumentace.

  //Pozice vrcholu gl_Position by měla být vypočtena pronásobením cameraProjectionView*model*pos.
  outVertex.gl_Position = si.uniforms[0].m4 * si.uniforms[10+si.gl_DrawID*5+0].m4 * glm::vec4(inVertex.attributes[0].v3, 1.f);
  //Pozice by se měla pronásobit modelovou maticí "m*glm::vec4(pos,1.f)", aby se ztransformovala do world-space.
  outVertex.attributes[0].v3 = si.uniforms[10+si.gl_DrawID*5+0].m4 * glm::vec4(inVertex.attributes[0].v3, 1.f); //pozice ve world space
  //Normála by se měla pronásobit inverzní transponovanou modelovou maticí "itm*glm::vec4(nor,0.f)" aby se dostala do world-space.
  outVertex.attributes[1].v3 = si.uniforms[10+si.gl_DrawID*5+1].m4 * glm::vec4(inVertex.attributes[1].v3, 0.f); //normala ve world space
  //Texturovací souřadnice se pouze přepošlou.
  outVertex.attributes[2].v2 = inVertex.attributes[2].v2; //tex. koordinaty
  //Pozice vrcholu v prostoru clip-space prostoru světla pro stíny by se měla vypočítat lightProjectionView*model*pos.
  outVertex.attributes[3].v4 = si.uniforms[3].m4 * si.uniforms[10+si.gl_DrawID*5+0].m4 * glm::vec4(inVertex.attributes[0].v3, 1.f);
}
//! [drawModel_vs]

#include<iostream>

/**
 * @brief This functionrepresents fragment shader of texture rendering method.
 *
 * @param outFragment output fragment
 * @param inFragment input fragment
 * @param si shader interface
 */
//! [drawModel_fs]
void drawModel_fragmentShader(OutFragment&outFragment,InFragment const&inFragment,ShaderInterface const&si){
  (void)outFragment;
  (void)inFragment;
  (void)si;
  /// \todo Tato funkce reprezentujte fragment shader.<br>
  /// Vaším úkolem je správně obarvit fragmenty a osvětlit je pomocí lambertova osvětlovacího modelu.
  /// Bližší informace jsou uvedeny na hlavní stránce dokumentace.
  
  auto pozice = inFragment.attributes[0].v3;
  auto nor = inFragment.attributes[1].v3;
  auto UV = inFragment.attributes[2].v2;
  auto clipPozice = inFragment.attributes[3].v4;
  auto N=glm::normalize(nor);
  auto doubleSided = si.uniforms[10+si.gl_DrawID*5+4].v1;
  glm::vec4 dC; //barva povrchu nebo textura
  glm::vec3 ambientLightColor = si.uniforms[7].v3;
  glm::vec3 lightColor = si.uniforms[8].v3;

  if(doubleSided && glm::dot(nor, pozice - si.uniforms[2].v3) >= 0)
    nor = -nor;
  
  // textura nebo barva   
  if(si.uniforms[10+si.gl_DrawID*5+3].i1 > -1) {
    dC = read_texture(si.textures[si.uniforms[10+si.gl_DrawID*5+3].i1], UV);
  } else dC = si.uniforms[10+si.gl_DrawID*5+2].v4;

  auto L = glm::normalize(pozice - si.uniforms[1].v3);
  float dF = glm::clamp(glm::dot(L,N),0.f,1.f);

  glm::vec3 aL = glm::vec3(dC) * ambientLightColor;
  glm::vec3 dL = glm::vec3(dC) * lightColor * dF;

  auto color = aL+dL;

  outFragment.gl_FragColor = glm::vec4(color, dC.a);

  if(dC.a < 0.5f)
    outFragment.discard = true;
}
//! [drawModel_fs]


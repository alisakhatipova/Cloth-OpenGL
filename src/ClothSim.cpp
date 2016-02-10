#include "ClothSim.h"
#include <cstdint>
#include <cmath>
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ClothMeshData CreateTest2Vertices()
{
  ClothMeshData mdata;
  int n = 10;
  int nsqr = n*n;
  float delta = 0.09f;
  float edgeLength = delta;
  float initialHardnes = 60;
  mdata.vertPos0.resize(nsqr);
  mdata.vertVel0.resize(nsqr);
  mdata.vertPos1.resize(nsqr);
  mdata.vertVel1.resize(nsqr);
  mdata.vertForces.resize(nsqr);
  mdata.vertMassInv.resize(nsqr);
  mdata.texCoord.resize(nsqr);
  mdata.vertNormals.resize(nsqr);
  mdata.pushed = 0;
  for (int i = 0; i < n; ++i)
	  mdata.vertMassInv[i] = 1.0f/1e20f; 
  for (int i = n; i < nsqr; ++i)
	  mdata.vertMassInv[i] = 0.1f;
  float deltacoord = 1.0f/(n - 1);
  for (int i = 0; i < n; ++i)
	  for (int j = 0; j < n; ++j){
		  mdata.vertPos0[i * n + j] = float4((j + 1) * delta, (i + 1) * delta, 0, 1);
		  mdata.texCoord[i * n + j] = float2(j * deltacoord, i *deltacoord);
		  mdata.vertVel0[i * n + j] = float4(0, 0, 0, 0);
	  }

  mdata.vertPos1 = mdata.vertPos0;
  mdata.vertVel1 = mdata.vertVel0;
  for (int i = 0; i < nsqr; i++) {
	for (int j = i + 1; j < nsqr; j++){
		float4 vA = mdata.vertPos0[i];
		float4 vB = mdata.vertPos0[j];
		float dist = length(vA-vB);
		if (dist < 2 * sqrtf(2.0f)*edgeLength){
			float hardness = initialHardnes * (edgeLength / dist);
			mdata.edgeIndices.push_back(i);
			mdata.edgeIndices.push_back(j);
			mdata.edgeHardness.push_back(hardness);
			mdata.edgeInitialLen.push_back(dist);
		}
	}
 }
  for (int i = 0; i < n - 1; i++) 
	for (int j = 0; j < n - 1; ++j){
	  mdata.edgeIndicesTriangl.push_back(i * n + j);
	  mdata.edgeIndicesTriangl.push_back(i * n + j + 1);
	  mdata.edgeIndicesTriangl.push_back((i + 1) * n + j);
	  mdata.edgeIndicesTriangl.push_back((i + 1) * n + j + 1);
	  mdata.edgeIndicesTriangl.push_back((i + 1)* n + j);
	  mdata.edgeIndicesTriangl.push_back(i * n + j + 1);
  }
  mdata.g_wind = float4(0, 0, 0.01, 0);

  for (int i = 0; i < mdata.vertNormals.size(); ++i) mdata.vertNormals[i] = float3(0, 0, 0);

  for (int i = 0; i < mdata.edgeIndicesTriangl.size()/3; ++i){
	  int ind1 = mdata.edgeIndicesTriangl[3*i], ind2 = mdata.edgeIndicesTriangl[3*i + 1], ind3 = mdata.edgeIndicesTriangl[3*i + 2];
	  float4 vec1 = mdata.vertPos0[ind1];
	  float4 vec2 = mdata.vertPos0[ind2];
	  float4 vec3 = mdata.vertPos0[ind3];
	  /*mdata.vertNormals[ind1] += cross_product( vec3 - vec1, vec2 - vec1);
	  mdata.vertNormals[ind2] += cross_product( vec1 - vec2, vec3 - vec2);
	  mdata.vertNormals[ind3] += cross_product( vec2 - vec3, vec1 - vec3);
	  */
	  mdata.vertNormals[ind1] += cross_product( vec2 - vec1,vec3 - vec1);
	  mdata.vertNormals[ind2] += cross_product(vec3 - vec2,  vec1 - vec2);
	  mdata.vertNormals[ind3] += cross_product(  vec1 - vec3, vec2 - vec3);
  }
  for (int i = 0; i < mdata.vertNormals.size(); ++i) mdata.vertNormals[i] = normalize(mdata.vertNormals[i]);
 // for (int i = 0; i < mdata.vertNormals.size(); ++i) std::cout << mdata.vertNormals[i].z <<std::endl;
  // you can use any intermediate mesh representation or load data to GPU (in VBOs) here immediately.                              <<===== !!!!!!!!!!!!!!!!!!

  // create graphics mesh; SimpleMesh uses GLUS Shape to store geometry; 
  // we copy data to GLUS Shape, and then these data will be copyed later from GLUS shape to GPU 
  //
  mdata.pMesh = std::make_shared<SimpleMesh>();
  mdata.pTris = std::make_shared<SimpleMesh>();
  GLUSshape& shape = mdata.pMesh->m_glusShape;

  shape.numberVertices = mdata.vertPos0.size();
  shape.numberIndices  = mdata.edgeIndices.size();

  shape.vertices  = (GLUSfloat*)malloc(4 * shape.numberVertices * sizeof(GLUSfloat));
  shape.indices   = (GLUSuint*) malloc(shape.numberIndices * sizeof(GLUSuint));

  memcpy(shape.vertices, &mdata.vertPos0[0], sizeof(float) * 4 * shape.numberVertices);
  memcpy(shape.indices, &mdata.edgeIndices[0], sizeof(int) * shape.numberIndices);

  // for tri mesh you will need normals, texCoords and different indices
  // 
  GLUSshape& shape1 = mdata.pTris->m_glusShape;
  shape1.numberVertices = mdata.vertPos0.size();
  shape1.numberIndices  = mdata.edgeIndicesTriangl.size();
  
  shape1.vertices  = (GLUSfloat*)malloc(4 * shape1.numberVertices * sizeof(GLUSfloat));
  shape1.indices   = (GLUSuint*) malloc(shape1.numberIndices * sizeof(GLUSuint));
  shape1.texCoords =  (GLUSfloat*)malloc(2 * shape1.numberVertices * sizeof(GLUSfloat));
  shape1.normals =  (GLUSfloat*)malloc(3 * shape1.numberVertices * sizeof(GLUSfloat));

  memcpy(shape1.vertices, &mdata.vertPos0[0], sizeof(float) * 4 * shape1.numberVertices);
  memcpy(shape1.indices, &mdata.edgeIndicesTriangl[0], sizeof(int) * shape1.numberIndices);
  memcpy(shape1.texCoords, &mdata.texCoord[0], sizeof(float) * 2 * shape1.numberVertices);  
  memcpy(shape1.normals, &mdata.vertNormals[0], sizeof(float) * 3 * shape1.numberVertices); 
  return mdata;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
float3 cross_product(float4 a, float4 b){
	float3 q, w, res ;
	q.x = a.x;
	q.y = a.y;
	q.z = a.z;

	w.x = b.x;
	w.y = b.y;
	w.z = b.z;
	res = cross(q, w);
	res = normalize(res);
	return res;
}
void ClothMeshData::updatePositionsGPU()
{
  if (pMesh == nullptr)
    return;
   
   GLUSshape& shape = pMesh->m_glusShape;
   glBindBuffer(GL_ARRAY_BUFFER, pMesh->m_vertexPosBufferObject);                                      CHECK_GL_ERRORS;
   glBufferSubData(GL_ARRAY_BUFFER, 0, shape.numberVertices * 4 * sizeof(GLfloat), (GLfloat*)shape.vertices);  CHECK_GL_ERRORS;
   
   GLUSshape& shape1 = pTris->m_glusShape;
   glBindBuffer(GL_ARRAY_BUFFER, pTris->m_vertexPosBufferObject);                                      CHECK_GL_ERRORS;
   glBufferSubData(GL_ARRAY_BUFFER, 0, shape1.numberVertices * 4 * sizeof(GLfloat), (GLfloat*)shape1.vertices);  CHECK_GL_ERRORS; 

   
  // copy current vertex positition to positions VBO
 
}

void ClothMeshData::updateNormalsGPU()
{
  if (pMesh == nullptr || this->vertNormals.size() == 0)
    return;
   GLUSshape& shape1 = pTris->m_glusShape;
   glBindBuffer(GL_ARRAY_BUFFER, pTris->m_vertexNormBufferObject);                                      CHECK_GL_ERRORS;
   glBufferSubData(GL_ARRAY_BUFFER, 0, shape1.numberVertices * 3 * sizeof(GLfloat), (GLfloat*)shape1.normals);  CHECK_GL_ERRORS; 
   
  // copy current recalculated normals to appropriate VBO on GPU

}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void SimStep(ClothMeshData* pMesh, float delta_t)
{
 
  // get in and out pointers
  //
  float4* inVertPos  = pMesh->pinPong ? &pMesh->vertPos1[0] : &pMesh->vertPos0[0];
  float4* inVertVel  = pMesh->pinPong ? &pMesh->vertVel1[0] : &pMesh->vertVel0[0];

  float4* outVertPos = pMesh->pinPong ? &pMesh->vertPos0[0] : &pMesh->vertPos1[0];
  float4* outVertVel = pMesh->pinPong ? &pMesh->vertVel0[0] : &pMesh->vertVel1[0];

  // accumulate forces first
  //
  float4 g = float4(0, -0.1, 0, 0);
  for (size_t i = 0; i < pMesh->vertForces.size(); i++) {// clear all forces
	  pMesh->vertForces[i] = g + pMesh->g_wind;
	  if (pMesh->pushed){
		  pMesh->vertForces[i] += float4(0, 0,1.5,0);
	  }
  }
  
 pMesh->pushed = 0;
  for (int connectId = 0; connectId < pMesh->connectionNumber(); connectId++)
  {
	  int i_cur = pMesh->edgeIndices[connectId *2], j_cur = pMesh->edgeIndices[connectId*2 + 1];
	  float4 vec_i = normalize(inVertPos[j_cur] - inVertPos[i_cur]), vec_j = normalize(inVertPos[i_cur] - inVertPos[j_cur]);
	  float dist = length(inVertPos[j_cur] - inVertPos[i_cur]);
	  float delta_l= dist - pMesh->edgeInitialLen[connectId];
	  float4 accel_i =vec_i * delta_l * pMesh->edgeHardness[connectId]*pMesh->vertMassInv[i_cur];
	  float4 accel_j =vec_j * delta_l * pMesh->edgeHardness[connectId]*pMesh->vertMassInv[j_cur];
	  pMesh->vertForces[i_cur] += accel_i;
	  pMesh->vertForces[j_cur] += accel_j;
  }

  // update positions and velocity
  //
  int n = 10;
  for (size_t i = 0; i < pMesh->vertPos0.size(); i++)
  {
	  if ( i < n) continue;
	 outVertVel[i] = inVertVel[i] + pMesh->vertForces[i] * delta_t;
	 outVertPos[i] = inVertPos[i] +  (inVertVel[i] + pMesh->vertForces[i] * delta_t) * delta_t +  pMesh->vertForces[i] * delta_t * delta_t/2;

  }
  inVertVel = outVertVel;
  inVertPos = outVertPos;
  GLUSshape& shape = pMesh->pMesh->m_glusShape;
  memcpy(shape.vertices, &outVertPos[0], sizeof(float) * 4 * shape.numberVertices);
  GLUSshape& shape1 = pMesh->pTris->m_glusShape;
  memcpy(shape1.vertices, &outVertPos[0], sizeof(float) * 4 * shape1.numberVertices); 
  pMesh->pinPong = !pMesh->pinPong; // swap pointers for next sim step
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void RecalculateNormals(ClothMeshData* pMesh)
{
   for (int i = 0; i < pMesh->vertNormals.size(); ++i) pMesh->vertNormals[i] = float3(0, 0, 0);

  for (int i = 0; i < pMesh->edgeIndicesTriangl.size()/3; ++i){
	  int ind1 = pMesh->edgeIndicesTriangl[3*i], ind2 = pMesh->edgeIndicesTriangl[3*i + 1], ind3 = pMesh->edgeIndicesTriangl[3*i + 2];
	  float4 vec1 = pMesh->vertPos0[ind1];
	  float4 vec2 = pMesh->vertPos0[ind2];
	  float4 vec3 = pMesh->vertPos0[ind3];
	  /*pMesh->vertNormals[ind1] += cross_product( vec3 - vec1, vec2 - vec1);
	  pMesh->vertNormals[ind2] += cross_product( vec1 - vec2, vec3 - vec2);
	  pMesh->vertNormals[ind3] += cross_product( vec2 - vec3, vec1 - vec3); */
	  pMesh->vertNormals[ind1] += cross_product( vec2 - vec1, vec3 - vec1);
	  pMesh->vertNormals[ind2] += cross_product( vec3 - vec2, vec1 - vec2);
	  pMesh->vertNormals[ind3] += cross_product( vec1 - vec3, vec2 - vec3);
  }
  for (int i = 0; i < pMesh->vertNormals.size(); ++i) pMesh->vertNormals[i] = normalize(pMesh->vertNormals[i]);
 GLUSshape& shape1 = pMesh->pTris->m_glusShape;
 float3* vertNorm  =  &pMesh->vertNormals[0];
  memcpy(shape1.normals, &vertNorm[0], sizeof(float) * 3 * shape1.numberVertices);  
}


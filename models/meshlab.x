xof 0303txt 0032
template Vector {
 <3d82ab5e-62da-11cf-ab39-0020af71e433>
 FLOAT x;
 FLOAT y;
 FLOAT z;
}

template MeshFace {
 <3d82ab5f-62da-11cf-ab39-0020af71e433>
 DWORD nFaceVertexIndices;
 array DWORD faceVertexIndices[nFaceVertexIndices];
}

template Mesh {
 <3d82ab44-62da-11cf-ab39-0020af71e433>
 DWORD nVertices;
 array Vector vertices[nVertices];
 DWORD nFaces;
 array MeshFace faces[nFaces];
 [...]
}

template MeshNormals {
 <f6f23f43-7686-11cf-8f52-0040333594a3>
 DWORD nNormals;
 array Vector normals[nNormals];
 DWORD nFaceNormals;
 array MeshFace faceNormals[nFaceNormals];
}

template VertexDuplicationIndices {
 <b8d65549-d7c9-4995-89cf-53a9a8b031e3>
 DWORD nIndices;
 DWORD nOriginalVertices;
 array DWORD indices[nIndices];
}


Mesh {
 6;
 1.000000;0.000000;0.000000;,
 0.000000;1.000000;0.000000;,
 0.000000;0.000000;1.000000;,
 -1.000000;0.000000;0.000000;,
 0.000000;-1.000000;0.000000;,
 0.000000;0.000000;-1.000000;;
 8;
 3;0,1,2;,
 3;0,2,4;,
 3;0,4,5;,
 3;0,5,1;,
 3;3,1,5;,
 3;3,5,4;,
 3;3,4,2;,
 3;3,2,1;;

 MeshNormals {
  6;
  1.000000;0.000000;0.000000;,
  0.000000;1.000000;0.000000;,
  0.000000;0.000000;1.000000;,
  -1.000000;0.000000;0.000000;,
  0.000000;-1.000000;0.000000;,
  0.000000;0.000000;-1.000000;;
  8;
  3;0,1,2;,
  3;0,2,4;,
  3;0,4,5;,
  3;0,5,1;,
  3;3,1,5;,
  3;3,5,4;,
  3;3,4,2;,
  3;3,2,1;;
 }

 VertexDuplicationIndices {
  6;
  6;
  0,
  1,
  2,
  3,
  4,
  5;
 }
}
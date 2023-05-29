//#pragma once
//
//
//#include "Mesh.h"
//#include "Shader.h"
//
//#include <mutex>
//
//
//namespace Magnefu
//{
//	class Model
//	{
//		using Cache = std::unordered_map<std::string, int>;
//
//		struct VertexCount
//		{
//			uint32_t posCount = 0;
//			uint32_t normalCount = 0;
//			uint32_t uvCount = 0;
//		};
//
//	public:
//		Model(std::string& filepath);
//		~Model();
//
//		void Init(Shader* shader);
//
//		void Draw(Shader* shader);
//		void OnImGUIRender();
//		//void ClearFromCache();
//
//	public:
//		std::string m_Filepath;
//
//		std::vector<std::unique_ptr<Mesh>> m_Meshes;
//
//	private:
//		void BindTextures();
//		void UnbindTextures();
//		void SetShaderUniforms(Shader* shader);
//
//		void ParseMaterial(const std::string& filepath, std::vector<SubMaterialStream>& ss);
//		void LoadMesh(std::vector<Maths::vec3>& tempPositions, std::vector<Maths::vec3>& tempNormals, std::vector<Maths::vec2>& tempTexCoords, std::vector<Face>& tempFaces);
//
//		Maths::vec3 GetVertexData(std::string& line, int elementCount);
//
//		std::array<Maths::vec3i, 4> GetFaceData(std::string& line, uint32_t& vertexCount);
//
//		// NEW
//
//		std::vector<Material*> m_Materials;
//	};
//}
//

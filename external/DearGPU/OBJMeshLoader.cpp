#include "OBJMeshLoader.h"
#include "CommonFileSystem.h"
#include "LineReader.h"

OBJMeshLoader::OBJMeshLoader()
{
	Materials.reserve(10);

	// verify ParseValue float parsing works for large fractional parts
	{
		float Value;

		const char* TestInput = "0.123456789123456789123456789";
		uint8* p = (uint8*)TestInput;

		ParseValue(p, Value);

		assert(Value == 0.123456789123456789123456789f);
	}
}

bool OBJMeshLoader::Load(const wchar_t* FileName, IIndexedMeshSink &Sink, EOBJLoaderFlags Flags, float Scale)
{
	UINT Positions;
	UINT Normals;
	UINT UVs;
	
	if(!LoadMeshPass1(FileName, Sink, Positions, Normals, UVs))
	{
		Sink.OnFailed("LoadMeshPass1");
		return false;
	}

	if(Flags & OLF_NoImportNormals)
	{
		Normals = 0;
	}

	// currently we require normals
//	assert(Normals);

	std::wstring Path = ExtractPathW(FileName);
	std::wstring FullMTLFilePath = Path + L"/" + MTLLibName;

	LoadMTL(FullMTLFilePath.c_str());
//	if(LoadMTL())
//	{
//		return false;
//	}

	Sink.OnPrepare(Positions, Normals, UVs, (uint32)Materials.size());

	// all materials
	{
		std::vector<COBJMeshMaterial>::const_iterator it, end = Materials.end();
		UINT MaterialId = 0;

		for(it = Materials.begin(); it!=end; ++it)
		{
			const COBJMeshMaterial &ref = *it;

			Sink.SetMaterial(MaterialId++, ref);
		}
	}

	if(!LoadMeshPass2(FileName, Sink, Flags, Scale))
	{
		Sink.OnFailed("LoadMeshPass2");
		return false;
	}

	return true;
}

COBJMeshMaterial &OBJMeshLoader::GetOBJMaterial(uint32 SubSetId)
{
	assert(SubSetId < Materials.size());

	return Materials[SubSetId];
}

uint32 OBJMeshLoader::GetOBJMaterialCount() const
{
	return (uint32)Materials.size();
}

void OBJMeshLoader::ParseWhiteSpace(uint8* &p)
{
	while(*p !=0 && *p <= ' ')
	{
		++p;
	}
}

void OBJMeshLoader::ParseFileName(uint8* &p, std::wstring &Out)
{
	Out.clear();

	while(*p > ' ')
	{
		Out += (wchar_t)*p++;
	}
}

void OBJMeshLoader::ParseName(uint8* &p, std::string &Out)
{
	Out.clear();

	while(*p > ' ')
	{
		Out += *p++;
	}
}

bool OBJMeshLoader::IsNameCharacter(uint8 Value)
{
	return (Value >= 'a' && Value <= 'z') || (Value >= 'A' && Value <= 'Z') || Value == '_';
}

bool OBJMeshLoader::ParseName(uint8* &p, const char *Name)
{
	size_t Length = strlen(Name);
	// can be optimized
	if(_strnicmp(Name, (char *)p, Length)==0)
	{
		// in order to not only recognize the start of a name but also that the source name ends there
		if(!IsNameCharacter(p[Length]))
		{
			p += Length;
			ParseWhiteSpace(p);
			return true;
		}
	}

	return false;
}

bool OBJMeshLoader::ParseValue(uint8* &p, int &outValue)
{
	uint8 *Backup = p;
	bool bNegate = false;

	if(*p == '-')
	{
		bNegate = true;
		++p;
	}

	if(*p < '0' || *p > '9')
	{
		p = Backup;
		return false;
	}

	outValue = 0;

	while(*p >= '0' && *p <= '9')
	{
		outValue = outValue * 10 + (*p - '0');

		++p;
	}

	if(bNegate)
	{
		outValue = -outValue;
	}

	ParseWhiteSpace(p);
	return true;
}


bool OBJMeshLoader::ParseValue(uint8* &p, float &outValue)
{
	uint8 *Backup = p;
	bool bNegate = false;

	if(*p == '-')
	{
		bNegate = true;
		++p;
	}

	if((*p < '0' || *p > '9') && *p != '.')
	{
		p = Backup;
		return false;
	}

	outValue = 0;

	// integer part
	while(*p >= '0' && *p <= '9')
	{
		outValue = outValue * 10 + (float)(*p - '0');

		++p;
	}

	if(*p == '.')
	{
		// fractional part
		++p;

		float Frac = 0.1f;

		while(*p >= '0' && *p <= '9')
		{
			outValue += Frac * (*p - '0');

			++p;
			Frac *= 0.1f;
		}
	}

	if(bNegate)
	{
		outValue = -outValue;
	}

	if(*p == 'e')
	{
		++p;
		bool bNegateExp = false;
		float Exponent = 0;
		if(*p == '-')
		{
			bNegateExp = true;
			++p;
		}

		while((*p >= '0' && *p <= '9'))
		{
			Exponent = Exponent * 10 + (float)(*p - '0');
			++p;
		}
		if(bNegateExp)
		{
			Exponent = -Exponent;
		}

		outValue *= powf(10, Exponent);
	}

	ParseWhiteSpace(p);
	return true;
}

bool OBJMeshLoader::ParseValue(uint8* &p, glm::vec3 &outValue)
{
	uint8 *Backup = p;

	if(ParseValue(p, outValue.x)
	&& ParseValue(p, outValue.y)
	&& ParseValue(p, outValue.z))
	{
		return true;
	}

	p = Backup;
	return false;
}

bool OBJMeshLoader::ParseValue(uint8* &p, glm::vec2 &outValue)
{
	uint8 *Backup = p;

	if(ParseValue(p, outValue.x)
	&& ParseValue(p, outValue.y))
	{
		return true;
	}

	p = Backup;
	return false;
}

bool OBJMeshLoader::LoadMeshPass1(const wchar_t*FileName, IIndexedMeshSink &Sink, UINT &Positions, UINT &Normals, UINT &UVs)
{
	Positions = 0;
	Normals = 0;
	UVs = 0;

	CLineReader LineReader;

	uint32 MaterialId = 0;

	if(LineReader.Open(FileName))
	{
		while(!LineReader.EndOfFile())
		{
			char Line[256];

			LineReader.GetLine(Line,sizeof(Line));

			uint8 *p = (uint8*)Line;

			ParseWhiteSpace(p);

//			OutputDebugString(_T("'"));
//			OutputDebugStringA(p);
//			OutputDebugString(_T("'\n"));
			if(ParseName(p, "usemtl"))
			{
				std::string MatName;
				ParseName(p, MatName);

				MaterialId = FindOrCreateMaterialId(MatName);
			}
			if(ParseName(p, "mtllib"))
			{
				if(*p == '.' && p[1] == '/')
				{
					p += 2;
				}

				ParseFileName(p, MTLLibName);
			}
			else if(ParseName(p, "v"))
			{
				++Positions;
			}
			else if(ParseName(p, "vt"))
			{
				++UVs;
			}
			else if(ParseName(p, "vn"))
			{
				++Normals;
			}
			else if(ParseName(p, "f"))
			{
				if(Materials.empty())
				{
					MaterialId = FindOrCreateMaterialId("default");
				}

				UINT VertexInThatLine = 0;

				while(*p)		// todo: endless loop bug with OBJ not ending in empty line
				{
					int Value;

					if(ParseValue(p, Value))
					{
						++VertexInThatLine;

						while(*p == '/')
						{
							++p;
							ParseWhiteSpace(p);
							ParseValue(p, Value);
						}
					}
				}

				Sink.PreparePoly(VertexInThatLine, MaterialId);
			}


		}
	}
	else 
	{
		return false;
	}

	return true;
}

UINT OBJMeshLoader::ComputeAbsoluteIndex(int RelOrAbsIndex, UINT CurrentAmount)
{
	if(RelOrAbsIndex < 0)
	{
		return CurrentAmount + RelOrAbsIndex;
	}
	else
	{
		assert(RelOrAbsIndex);	// cannot be 0

		return (UINT)RelOrAbsIndex-1;
	}
}

bool OBJMeshLoader::LoadMeshPass2(const wchar_t*FileName, IIndexedMeshSink &Sink, EOBJLoaderFlags Flags, float Scale)
{
	UINT PositionId = 0;
	UINT NormalId = 0;
	UINT UVId = 0;

	uint32 MaterialId = 0;

	CLineReader LineReader;

	if(LineReader.Open(FileName))
	{
		while(!LineReader.EndOfFile())
		{
			char Line[256];

			LineReader.GetLine(Line, sizeof(Line));

			uint8 *p = (uint8*)Line;

			ParseWhiteSpace(p);

//			OutputDebugString(_T("'"));
//			OutputDebugStringA(p);
//			OutputDebugString(_T("'\n"));

			if(ParseName(p, "usemtl"))
			{
				std::string MatName;
				ParseName(p, MatName);

				MaterialId = FindOrCreateMaterialId(MatName);
			}
			if(ParseName(p, "v"))
			{
                glm::vec3 Value;

				if(!ParseValue(p, Value))
				{
					Sink.OnFailed("p");
					return false;
				}

				if(Flags & OLF_Reorient)
				{
					Value = glm::vec3(Value.x, Value.z, -Value.y);
				}

				Value.x *= Scale;
				Value.y *= Scale;
				Value.z *= Scale;
				
				Sink.SetPos(PositionId++, Value);
			}
			else if(ParseName(p, "vt"))
			{
                glm::vec2 Value;

				if(!ParseValue(p, Value))
				{
					Sink.OnFailed("vt");
					return false;
				}

				Value.y = 1.0f - Value.y;

				Sink.SetUV(UVId++, Value);
			}
			else if(ParseName(p, "vn"))
			{
                glm::vec3 Value;

				if(!ParseValue(p, Value))
				{
					Sink.OnFailed("vn");
					return false;
				}

				if(Flags & OLF_Reorient)
				{
					Value = glm::vec3(Value.x, Value.z, -Value.y);
				}

				if(!(Flags & OLF_NoImportNormals))
				{
					Sink.SetNormal(NormalId++, Value);
				}
			}
			else if(ParseName(p, "f"))
			{
				UINT VertexId = 0;	// 0..2

				const static int MaxPolyVertexCount = 32;
				SIndexedVertex Vertices[MaxPolyVertexCount]; 

				while(*p)
				{
					int Value;

					if(!ParseValue(p, Value))
					{
						Sink.OnFailed("f v");
						return false;
					}
					
					Vertices[VertexId].Reset();
					Vertices[VertexId].PositionIndex = ComputeAbsoluteIndex(Value, PositionId);
					// in case no normals are specified we at least have unique indices for tangentspace calculation
					// (only requires indices, no actual normals)
					Vertices[VertexId].NormalIndex = Vertices[VertexId].PositionIndex;

					if(*p == '/')
					{
						++p; ParseWhiteSpace(p);

						if(UVId)
						{
							if(ParseValue(p, Value))
							{
								Vertices[VertexId].UVIndex = ComputeAbsoluteIndex(Value, UVId);
							}
						}

						if(*p == '/')
						{
							++p; ParseWhiteSpace(p);

							if(ParseValue(p, Value))
							{
								Vertices[VertexId].NormalIndex = ComputeAbsoluteIndex(Value, NormalId);
							}
						}
					}

					if(Flags & OLF_NoImportNormals)
					{
						Vertices[VertexId].NormalIndex = 0;
					}

					++VertexId;

					if(VertexId >= MaxPolyVertexCount)
					{
						// error, we silently ignore ngones with too many sides
						assert(0);
						break;
					}
				}

				// error, we silently ignore ngones with too few sides
				if(VertexId >= 3)
				{
					Sink.AddPoly(Vertices, VertexId, MaterialId);
				}
			}
		}
	}

	return true;
}

uint32 OBJMeshLoader::FindOrCreateMaterialId(const std::string &MatName)
{
	std::vector<COBJMeshMaterial>::const_iterator it, end = Materials.end();

	uint32 ret = 0;
	for(it = Materials.begin(); it != end; ++it, ++ret)
	{
		if(it->MaterialName == MatName)
		{
			return ret;
		}
	}

	Materials.push_back(COBJMeshMaterial());

	Materials.back().MaterialName = MatName;

	return (uint32)(Materials.size() - 1);
}

bool OBJMeshLoader::LoadMTL(const wchar_t*FileName)
{
	CLineReader LineReader;

	COBJMeshMaterial *OutMaterial = 0;

	if(LineReader.Open(FileName))
	{
		while(!LineReader.EndOfFile())
		{
			char Line[256];

			LineReader.GetLine(Line, sizeof(Line));

			uint8 *p = (uint8 *)Line;

			ParseWhiteSpace(p);

//			OutputDebugString(_T("'"));
//			OutputDebugStringA(p);
//			OutputDebugString(_T("'\n"));

			bool bError = false;

			if(ParseName(p, "newmtl"))
			{
				std::string MatName;
				ParseName(p, MatName);

				uint32 MaterialIndex = FindOrCreateMaterialId(MatName);

				OutMaterial = &Materials[MaterialIndex];
			}
			
			if(OutMaterial)
			{
				bError = false;

				if(ParseName(p, "Kd"))
				{
					bError = ParseValue(p, OutMaterial->DiffuseColor);	
				}
				else if(ParseName(p, "Ks"))
				{
					bError = ParseValue(p, OutMaterial->SpecularColor);	
				}
				else if (ParseName(p, "Ke"))
				{
					bError = ParseValue(p, OutMaterial->EmissiveColor);
				}
//				else if(ParseName(p, "Ka"))
//				{
//					bError = ParseValue(p, OutMaterial->AmbientColor);
//				}
				else if(ParseName(p, "Ns"))
				{
					bError = ParseValue(p, OutMaterial->SpecularPower);
				}
				else if(ParseName(p, "map_Kd"))
				{
					ParseFileName(p, OutMaterial->DiffuseTexture);
				}
				else if(ParseName(p, "map_Ks"))
				{
					ParseFileName(p, OutMaterial->SpecularTexture);
				}
				else if(ParseName(p, "bump"))
				{
					ParseFileName(p, OutMaterial->BumpTexture);
				}
			}
		}
	}

	return true;
}

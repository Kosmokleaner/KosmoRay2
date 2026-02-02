#pragma once
// copyright DearGPU
#include <map>
#include <string>

class CIniArchive {
 public:
  struct SValueEx {
    //		bool bLoadedFromFile = false;
    // comment line before value, may be empty, lines are separated by "\r\n"
    std::string CommentBlock;
    std::string Value;

    float GetFloat(float Default = 0.0f) const {
      float Out = Default;

      sscanf_s(Value.c_str(), "%f", &Out);

      return Out;
    }
    int GetInt(int Default = 0.0f) const {
      int Out = Default;

      sscanf_s(Value.c_str(), "%d", &Out);

      return Out;
    }
  };

//  bool Load(const TCHAR* pFileName);
//  bool Save(const TCHAR* pFileName);
  bool Load(const char* pFileName);
  bool Save(const char* pFileName);

  // @return 0 if not found
  SValueEx* GetByKey(std::string Key);

  // @param InCommentBlock may be 0, data is copied
  void AddKeyValue(const char* Key, const std::string& Value, const char* InCommentBlock = 0);
  void AddKeyValue(const char* Key, float Value, const char* InCommentBlock = 0);
  // also works with bool Value
  void AddKeyValue(const char* Key, int Value, const char* InCommentBlock = 0);

  bool IsLoading() const {
    return bIsLoading;
  }
  bool IsSaving() const {
    return !bIsLoading;
  }

 private:
  bool bIsLoading = false;

  // todo: support sections
  std::map<std::string, SValueEx> KeyValuePairs;
};

void Reflection(CIniArchive& archive, const char* Key, float& Value);
void Reflection(CIniArchive& archive, const char* Key, int& Value);
void Reflection(CIniArchive& archive, const char* Key, bool& Value);

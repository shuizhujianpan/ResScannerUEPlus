#include "CoreMinimal.h"
#include "TemplateHelper.hpp"
#include "Misc/CommandLine.h"

namespace ReplacePropertyHelper
{
	static bool HasPrroperty(UStruct* Field,const FString& FieldName,FProperty*& Property)
	{
		for(TFieldIterator<FProperty> PropertyIter(Field);PropertyIter;++PropertyIter)
		{
			if(PropertyIter->GetName().Equals(FieldName,ESearchCase::IgnoreCase))
			{
				Property = *PropertyIter;
				return true;
			}
		}
		return false;
	}

	template<typename T>
		static void ReplaceProperty(T& Struct, const TMap<FString, FString>& ParamsMap)
	{
		TSharedPtr<FJsonObject> DeserializeJsonObject;
		TemplateHelper::TSerializeStructAsJsonObject(Struct,DeserializeJsonObject);
		if (DeserializeJsonObject.IsValid())
		{
			TArray<FString> MapKeys;
			ParamsMap.GetKeys(MapKeys);

			for(const auto& key:MapKeys)
			{
				TArray<FString> BreakedDot;
				key.ParseIntoArray(BreakedDot,TEXT("."));
				if(BreakedDot.Num())
				{
					TSharedPtr<FJsonObject> JsonObject = DeserializeJsonObject;
					FProperty* Property;
					if(ReplacePropertyHelper::HasPrroperty(T::StaticStruct(),BreakedDot[0],Property))
					{
						for(int32 index=0;index<BreakedDot.Num()-1;++index)
						{
							JsonObject = JsonObject->GetObjectField(BreakedDot[index]);
						}

						if(JsonObject)
						{
							JsonObject->SetStringField(BreakedDot[BreakedDot.Num()-1],*ParamsMap.Find(key));
						}
					}
				}
			}
			TemplateHelper::TDeserializeJsonObjectAsStruct<T>(DeserializeJsonObject,Struct);
		}
	}

	static TMap<FString, FString> GetCommandLineParamsMap(const FString& CommandLine)
	{
		TMap<FString, FString> resault;
		TArray<FString> ParamsSwitchs, ParamsTokens;
		FCommandLine::Parse(*CommandLine, ParamsTokens, ParamsSwitchs);

		for (const auto& SwitchItem : ParamsSwitchs)
		{
			TArray<FString> SwitchArray;
			SwitchItem.ParseIntoArray(SwitchArray,TEXT("="),true);
			if(SwitchArray.Num()>1)
			{
				resault.Add(SwitchArray[0],SwitchArray[1].TrimQuotes());
			}
			else
			{
				resault.Add(SwitchArray[0],TEXT(""));
			}
		}
		return resault;
	}
	static TArray<FString> GetArrayElementByTokens(const FString& PropertyName, const TMap<FString, FString>& ParamsMap)
	{
		TArray<FString> Keys;
		ParamsMap.GetKeys(Keys);
		TArray<FString> ChildValues;
		for(const auto& Key:Keys)
		{
			if(Key.Equals(PropertyName,ESearchCase::IgnoreCase))
			{
				FString Value = *ParamsMap.Find(Key);
				Value.ParseIntoArray(ChildValues,TEXT(","));
				break;
			}
		}
		return ChildValues;
	};

	static TArray<FString> ParserParserFiltersByCommandline(const FString& Commandline,const FString& Token)
	{
		TArray<FString> result;
		TMap<FString, FString> KeyValues = GetCommandLineParamsMap(Commandline);
		if(KeyValues.Find(Token))
		{
			FString AddPakListInfo = *KeyValues.Find(Token);
			AddPakListInfo.ParseIntoArray(result,TEXT(","));
		}
		return result;
	}

	static TArray<FDirectoryPath> ParserFilters(const FString& Commandline,const FString& FilterName)
	{
		TArray<FDirectoryPath> Result;
		for(auto& FilterPath:ParserParserFiltersByCommandline(Commandline,FilterName))
		{
			FDirectoryPath Path;
			Path.Path = FilterPath;
			Result.Add(Path);
		}
		return Result;
	}
	static TArray<FSoftObjectPath> ParserAssets(const FString& Commandline,const FString& FilterName)
	{
		TArray<FSoftObjectPath> Result;
		for(auto& FilterPath:ParserParserFiltersByCommandline(Commandline,FString::Printf(TEXT("Add%s"),*FilterName)))
		{
			Result.Emplace(FilterPath);
		}
		return Result;
	}
}

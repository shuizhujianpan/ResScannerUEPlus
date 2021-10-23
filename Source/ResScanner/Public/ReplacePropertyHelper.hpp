#include "CoreMinimal.h"
#include "TemplateHelper.hpp"

namespace ReplacePropertyHelper
{
	static bool HasPrroperty(UStruct* Field,const FString& FieldName)
	{
		for(TFieldIterator<FProperty> PropertyIter(Field);PropertyIter;++PropertyIter)
		{
			if(PropertyIter->GetName().Equals(FieldName,ESearchCase::IgnoreCase))
			{
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
					if(ReplacePropertyHelper::HasPrroperty(T::StaticStruct(),BreakedDot[0]))
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
}

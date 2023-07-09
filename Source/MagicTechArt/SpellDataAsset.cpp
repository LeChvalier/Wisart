// Fill out your copyright notice in the Description page of Project Settings.


#include "SpellDataAsset.h"

#include "Json.h"
#include "JsonUtilities/Public/JsonObjectConverter.h"

bool USpellDataAsset::Save()
{
	const FString Filepath = FPaths::ProjectContentDir() + "JsonData/" + GetName() + ".json";
	FString JsonString;
	FSaveData SaveData;
	SaveData.SymbolTemplate = symbolTemplate;
	const TSharedPtr<FJsonObject> JsonObject = FJsonObjectConverter::UStructToJsonObject(SaveData);
	if (!FJsonSerializer::Serialize(JsonObject.ToSharedRef(), TJsonWriterFactory<>::Create(&JsonString)))
	    return false;
	
	FFileHelper::SaveStringToFile(JsonString, *Filepath);
	return true;
}

bool USpellDataAsset::Load()
{
	const FString Filepath = FPaths::ProjectContentDir() + "JsonData/" + GetName() + ".json";
	FString JsonString = "";
	if (!FFileHelper::LoadFileToString(JsonString, *Filepath))
		return false;
	
	TSharedPtr<FJsonObject> JsonObject = nullptr;
	if (!FJsonSerializer::Deserialize(TJsonReaderFactory<>::Create(JsonString), JsonObject))
		return false;
	
	FSaveData SaveData;
	if (!FJsonObjectConverter::JsonObjectToUStruct<FSaveData>(JsonObject.ToSharedRef(), &SaveData))
		return false;

	symbolTemplate = SaveData.SymbolTemplate;
	// Set the index maybe
	return true;
}





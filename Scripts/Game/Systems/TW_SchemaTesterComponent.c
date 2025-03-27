[EntityEditorProps(category: "GameScripted/TrainWreck", description: "Purely to test schemas", color: "0 0 255 255")]
class TW_SchemaTesterComponentClass : ScriptComponentClass {};

class TW_SchemaTesterComponent : ScriptComponent
{
	ref TW_SchemaManager<SpawnSettingsBase> manager = new TW_SchemaManager<SpawnSettingsBase>();
	
	override void OnPostInit(IEntity owner)
	{	
		manager.LoadSchemaFromPath("Schemas/TrainWreckSchema.json");
	}
};
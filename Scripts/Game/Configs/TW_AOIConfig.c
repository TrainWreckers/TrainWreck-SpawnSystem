[BaseContainerProps(configRoot: true)]
class TW_AOIConfig
{
	[Attribute("AOI Name", UIWidgets.Auto, category: "Basic Info")]
	private string _name;
	
	[Attribute("50", UIWidgets.Slider, category: "Size", params: "10 100 5")]
	private float _minRadius;
	
	[Attribute("100", UIWidgets.Slider, category: "Size", params: "10 100 5")]
	private float _maxRadius;
	
	[Attribute("500", UIWidgets.Slider, category: "Search", params: "10 1000 5")]
	private float _searchRadius;
	
	[Attribute("50", UIWidgets.Slider, category: "Search", params: "10 5000 5")]
	private float _minDistanceFromCenter;
	
	[Attribute("0", UIWidgets.CheckBox, category: "Basic Info")]
	private bool _hasPlayerSpawn;
	
	[Attribute("", UIWidgets.ComboBox, "Valid areas for this area to spawn in", "", ParamEnumArray.FromEnum(EMapDescriptorType))]
	private ref array<EMapDescriptorType> _spawnableAreas;
	
	string GetName() { return _name; }
	float GetMinRadius() { return _minRadius; }
	float GetMaxRadius() { return _maxRadius; }
	bool HasPlayerSpawn() { return _hasPlayerSpawn; }
	float GetSearchRadius() { return _searchRadius; }	
	float GetDistanceFromCenter() { return _minDistanceFromCenter; }
	
	array<EMapDescriptorType> GetSpawnableAreas() { return _spawnableAreas; }
	bool IsValidSpawnableArea(EMapDescriptorType areaType) { return _spawnableAreas.Contains(areaType); }
	
	[Attribute("", UIWidgets.ResourceNamePicker, params: "conf class=TW_FactionCompositions")]
	private ref TW_FactionCompositions _compositions;
	
	TW_FactionCompositions GetCompositions() { return _compositions; }
	
	[Attribute("", UIWidgets.Auto, params: "conf class=TW_MustSpawnPrefabConfig")]
	private ref array<ref TW_MustSpawnPrefabConfig> _mustSpawnPrefabs;	
	
	array<ref TW_MustSpawnPrefabConfig> GetMustSpawnPrefabConfigs() { return _mustSpawnPrefabs; }
};
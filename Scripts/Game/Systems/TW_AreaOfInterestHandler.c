//! Composition Size Type, where the integer value corresponds to its diameter
enum TW_CompositionSize
{
	SMALL = 8,
	MEDIUM = 15,
	LARGE = 23
};

enum TW_CompositionType
{
	SMALL = 0,
	MEDIUM = 1,
	LARGE = 2,
	WALLS = 3,
	BUNKERS = 4,
	NONE = 5
};

class TW_CompositionSpawnParameters
{
	int SmallCompositions;
	int MediumCompositions;
	int LargeCompositions;
	int DefensiveWalls;
	int DefensiveBunkers;
	
	ref TW_FactionCompositions Compositions;
	bool ShouldFailAfterRetries = false;
	
	float NextSearchPosition;
	int RetriesPerPlacement = 5;
	
	//! Center of area/location (like a town for instance)
	vector AreaCenter;
	
	//! The radius of said location where this area of interest should spawn around
	float AreaRadius;
	
	//! AOI should be at least this far from area center
	int MinimumDistanceFromAreaCenter = 50;
	
	void TW_CompositionSpawnParameters(TW_FactionCompositions settings)
	{
		if(!settings)
			Debug.Error("TW_FactionCompositions cannot be null");
		
		Compositions = settings;
		NextSearchPosition = 1.25;
	}
	
	TW_CompositionSpawnParameters Copy()
	{
		ref TW_CompositionSpawnParameters params = new TW_CompositionSpawnParameters(Compositions);
		params.SmallCompositions = SmallCompositions;
		params.MediumCompositions = MediumCompositions;
		params.LargeCompositions = LargeCompositions;
		params.DefensiveWalls = DefensiveWalls;
		params.DefensiveBunkers = DefensiveBunkers;
		
		params.ShouldFailAfterRetries = ShouldFailAfterRetries;
		params.NextSearchPosition = NextSearchPosition;
		params.AreaRadius = AreaRadius;
		params.AreaCenter = AreaCenter;
		params.MinimumDistanceFromAreaCenter = MinimumDistanceFromAreaCenter;
		
		return params;
	}
};

class TW_CompositionPlacementResult
{
	//! Prefab to spawn
	ResourceName Prefab;
	
	//! Size of composition
	TW_CompositionSize CompositionSize;
	
	//! Where the position was found
	vector Position;		
};

enum TW_AOIFailedReason
{
	//! Composition has failed to be placed
	FAILED_TO_PLACE_COMPOSITION,
	
	//! Was unable to locate a valid home for compositions
	FAILED_TO_FIT_COMPS_AT_AOI,
	
	//! Failed to find a suitable AOI
	FAILED_TO_FIND_SUITABLE_AOI,
	
	NONE
};

class TW_AreaOfInterestHandler
{
	//! Contains all successful placement positions for prefabs
	private ref array<ref TW_CompositionPlacementResult> _placements = {};
	
	//! List of all entities that have been placed
	private ref array<IEntity> _placedEntities = {};
	
	private SCR_BaseGameMode _gameMode;
	private MapEntity _mapManager;
	
	private ref ScriptInvoker<TW_CompositionPlacementResult> _onCompositionBasePlacementFailed = new ScriptInvoker<TW_CompositionPlacementResult>();
	private ref ScriptInvoker<TW_CompositionPlacementResult> _onCompositionBasePlacementSuccess = new ScriptInvoker<TW_CompositionPlacementResult>();
	private ref ScriptInvoker<TW_AOIFailedReason> _onAOIFailed = new ScriptInvoker<TW_AOIFailedReason>();
	
	ScriptInvoker<TW_AOIFailedReason> GetOnAOIFailed() { return _onAOIFailed; }
	ScriptInvoker<TW_CompositionPlacementResult> GetOnCompositionPlacementFailed() { return _onCompositionBasePlacementFailed; }
	ScriptInvoker<TW_CompositionPlacementResult> GetOnCompositionPlacementSuccess() { return _onCompositionBasePlacementSuccess; }
	
	//! The amount of things to spawn
	private ref TW_CompositionSpawnParameters _parameters;
	private ref TW_CompositionSpawnParameters _copyParameters;
	
	private static ref array<TW_CompositionType> s_CompTypes = {};
	
	const int SPAWN_DELAY = 500;
	const int PADDING = 10;
	
	private vector _previousPosition;
	private vector _areaOfInterestPosition;
	private int _currentAttempt;
	private int _aoiPlacementAttempt;
	private TW_AOIFailedReason _failedReason = TW_AOIFailedReason.NONE;
	
	static ResourceName SmallZone = "{588A3DAF0296B0C1}PrefabsEditable/Slots/Flat/E_SlotFlatSmall.et";
	static ResourceName MediumZone = "{4B1C4BDDF6683101}PrefabsEditable/Slots/Flat/E_SlotFlatMedium.et";
	static ResourceName LargeZone = "{055DDE93F47A6A1D}PrefabsEditable/Slots/Flat/E_SlotFlatLarge.et";
	
	void ~TW_AreaOfInterestHandler()
	{
		_onCompositionBasePlacementFailed.Clear();
		_onCompositionBasePlacementSuccess.Clear();
		_onAOIFailed.Clear();		
	}
	
	void Delete()
	{
		foreach(IEntity entity : _placedEntities)
		{
			if(entity)
				SCR_EntityHelper.DeleteEntityAndChildren(entity);
		}
		
		_placedEntities.Clear();
	}
	
	void TW_AreaOfInterestHandler(TW_CompositionSpawnParameters parameters)
	{	
		_parameters = parameters;
		
		if(!_parameters.Compositions)
		{
			Print("TrainWreck: TW_AreaOfInterestHandler -> parameters must have a valid Composition collection", LogLevel.ERROR);			
			return;
		}
		
		_copyParameters = _parameters.Copy();
		
		_mapManager = GetGame().GetMapManager();
		_gameMode = SCR_BaseGameMode.Cast(GetGame().GetGameMode());		
		
		if(s_CompTypes.IsEmpty())
			SCR_Enum.GetEnumValues(TW_CompositionType, s_CompTypes);
		
		// Set attempts here
		_currentAttempt = parameters.RetriesPerPlacement;
		_aoiPlacementAttempt = parameters.RetriesPerPlacement;
	}
	
	bool StartSpawn()
	{
		if(_aoiPlacementAttempt <= 0) 
			return false;
		
		_placements.Clear();
		_currentAttempt = _parameters.RetriesPerPlacement;		
		_aoiPlacementAttempt -= 1;		
		_areaOfInterestPosition = TW_Util.RandomPositionAround(_parameters.AreaCenter, _parameters.AreaRadius, _parameters.MinimumDistanceFromAreaCenter);
		_previousPosition = _areaOfInterestPosition;
		
		if(Query())
			SpawnNext();
		
		return !_placedEntities.IsEmpty();
	}
	
	private TW_CompositionSize GetSize(TW_CompositionType type)
	{
		switch(type)
		{
			case TW_CompositionType.SMALL: return TW_CompositionSize.SMALL;
			case TW_CompositionType.MEDIUM: return TW_CompositionSize.MEDIUM;
			case TW_CompositionType.LARGE: return TW_CompositionSize.LARGE;
		}
		
		return TW_CompositionSize.SMALL;
	}
	
	private void Fail(TW_AOIFailedReason reason)
	{
		_failedReason = reason;
		
		if(GetOnAOIFailed())
			GetOnAOIFailed().Invoke(reason);					
	}
	
	private bool Query()
	{		
		vector position = _areaOfInterestPosition;
		ref array<vector> positions = {};
		
		ref array<TW_CompositionType> types = {};
		SCR_Enum.GetEnumValues(TW_CompositionType, types);
				
		foreach(TW_CompositionType currentType : types)
		{							
			positions.Clear();
			string typeName = SCR_Enum.GetEnumName(TW_CompositionType, currentType);			
			TW_CompositionSize compSize = GetSize(currentType);
			int requiredCount = GetCountFor(currentType);						
			
			int positionAttempts = 0;			
			int radius = (_parameters.NextSearchPosition * compSize);
			int positionCount;
			
			positionCount = FindOpenAreasForCompositionType(position, radius, compSize, positions);
			
			while(positionCount <= 0 && positionAttempts < 10)
			{
				positionAttempts++;
				radius = (_parameters.NextSearchPosition * compSize) + (_parameters.NextSearchPosition * compSize * positionAttempts);
				positionCount = FindOpenAreasForCompositionType(position, radius, compSize, positions);	
			}
			
			PrintFormat("TrainWreck: %1 Requires '%2' positions, found '%3', Radius: %4", typeName, requiredCount, positionCount, radius);
			
			if(positionCount > 0)
			{
				for(int i = 0; i < requiredCount; i++)
				{
					if(positions.IsEmpty()) 
						break;										
					
					int randomIndex = positions.GetRandomIndex();
					vector pos = positions.Get(randomIndex);					
					positions.Remove(randomIndex);
					
					
					ref TW_CompositionPlacementResult result = new TW_CompositionPlacementResult();
					result.Position = pos;
					result.Prefab = GetCompositionPrefab(currentType);
					result.CompositionSize = compSize;
					
					_placements.Insert(result);
					
					if(GetOnCompositionPlacementSuccess())
						GetOnCompositionPlacementSuccess().Invoke(result);
				}
			}
			
			ClearCountFor(currentType);			
			currentType = GetNextSpawnType();
		}
		
		return !_placements.IsEmpty();
	}
	
	private void SpawnNext()
	{
		int count = _placements.Count();
		if(count == 0) return;
		
		ref TW_CompositionPlacementResult result = _placements.Get(0);		
		SpawnFromResult(result);
		
		for(int i = 1; i < count; i++)
		{
			vector previousPosition = result.Position;
			result = _placements.Get(i);
			
			float distance = vector.Distance(previousPosition, result.Position);
			
			if(distance <= (int)result.CompositionSize)
			{
				result.Position = TW_Util.RandomPositionAround(previousPosition, result.CompositionSize * 1.5 + PADDING, result.CompositionSize + PADDING);				
				EntitySpawnParams params = EntitySpawnParams();
				vector mat[4];
				mat[3] = result.Position;
				SCR_TerrainHelper.SnapToTerrain(mat, GetGame().GetWorld(), true);
				result.Position = mat[3];			
			}
			
			SpawnFromResult(result);
		}
		
		_placements.Clear();		
	}
	
	private TW_CompositionType GetNextSpawnType()
	{
		if(_parameters.SmallCompositions > 0)
		{
			if(_parameters.Compositions.HasSmallCompositions()) return TW_CompositionType.SMALL;
			else
			{
				_parameters.SmallCompositions = 0;
			}
		}
		
		if(_parameters.MediumCompositions > 0)
		{
			if(_parameters.Compositions.HasMediumCompositions()) return TW_CompositionType.MEDIUM;
			else
				_parameters.MediumCompositions = 0;
		}
		
		if(_parameters.LargeCompositions > 0)
		{
			if(_parameters.Compositions.HasLargeCompositions()) return TW_CompositionType.LARGE;
			else _parameters.LargeCompositions = 0;
		}
		
		if(_parameters.DefensiveWalls > 0)
		{
			if(_parameters.Compositions.HasDefensiveWalls()) return TW_CompositionType.WALLS;
			else _parameters.DefensiveWalls = 0;
		}
		
		if(_parameters.DefensiveBunkers > 0)
		{
			if(_parameters.Compositions.HasDefensiveBunkers()) return TW_CompositionType.BUNKERS;
			else _parameters.DefensiveBunkers = 0;
		}
				
		return TW_CompositionType.NONE;
	}
	
	private void Decrement(TW_CompositionType type)
	{
		switch(type)
		{
			case TW_CompositionType.SMALL:
			{
				_parameters.SmallCompositions -= 1;
				break;
			}
			
			case TW_CompositionType.MEDIUM:
			{
				_parameters.MediumCompositions -= 1;
				break;
			}
			
			case TW_CompositionType.LARGE:
			{
				_parameters.LargeCompositions -= 1;
				break;
			}
			
			case TW_CompositionType.WALLS: 
			{
				_parameters.DefensiveWalls -= 1;
				break;
			}
			
			case TW_CompositionType.BUNKERS:
			{
				_parameters.DefensiveWalls -= 1;
				break;
			}
		}		
	}
	
	private int GetCountFor(TW_CompositionType type)
	{
		switch(type)
		{
			case TW_CompositionType.SMALL: return _parameters.SmallCompositions;
			case TW_CompositionType.MEDIUM: return _parameters.MediumCompositions;
			case TW_CompositionType.LARGE: return _parameters.LargeCompositions;
			case TW_CompositionType.WALLS: return _parameters.DefensiveWalls;
			case TW_CompositionType.BUNKERS: return _parameters.DefensiveBunkers;			
		}
		
		return 0;
	}
	
	private void ClearCountFor(TW_CompositionType type)
	{
		switch(type)
		{
			case TW_CompositionType.SMALL: 
			{
				_parameters.SmallCompositions = 0;
				break;
			}
			
			case TW_CompositionType.MEDIUM:
			{
				_parameters.MediumCompositions = 0;
				break;
			}
			
			case TW_CompositionType.LARGE: 
			{
				_parameters.LargeCompositions = 0;
				break;
			}
			
			case TW_CompositionType.WALLS: 
			{
				_parameters.DefensiveWalls = 0;
				break;
			}
			
			case TW_CompositionType.BUNKERS: 
			{
				_parameters.DefensiveWalls = 0;
				break;
			}
		}
	}
	
	private ResourceName GetCompositionPrefab(TW_CompositionType type)
	{
		switch(type)
		{
			case TW_CompositionType.SMALL: return _parameters.Compositions.GetRandomSmallComposition();
			case TW_CompositionType.MEDIUM: return _parameters.Compositions.GetRandomMediumComposition();
			case TW_CompositionType.LARGE: return _parameters.Compositions.GetRandomLargeComposition();
			case TW_CompositionType.WALLS: return _parameters.Compositions.GetRandomDefensiveWall();
			case TW_CompositionType.BUNKERS: return _parameters.Compositions.GetRandomDefensiveBunker();
		}
		
		return ResourceName.Empty;
	}
	
	bool SpawnFromResult(TW_CompositionPlacementResult result)
	{
		EntitySpawnParams params = EntitySpawnParams();
		vector mat[4];
		mat[3] = result.Position;
		
		params.Transform = mat;
		params.TransformMode = ETransformMode.WORLD;
		
		vector angles = Math3D.MatrixToAngles(params.Transform);
		angles[0] = Math.RandomFloat(0, 360);
		Math3D.AnglesToMatrix(angles, params.Transform);
		
		Resource resource = Resource.Load(result.Prefab);
		if(!resource.IsValid())
			return false;
		
		IEntity entity = GetGame().SpawnEntityPrefab(resource, GetGame().GetWorld(), params);
		
		if(!entity)
			return false;
		
		SCR_EditableEntityComponent editable = SCR_EditableEntityComponent.GetEditableEntity(entity);
		
		if(editable)
		{
			SCR_EditorPreviewParams previewParams = SCR_EditorPreviewParams.CreateParams(params.Transform, EEditorTransformVertical.TERRAIN);
			SCR_RefPreviewEntity.SpawnAndApplyReference(editable, previewParams);
			PrintFormat("TrainWreck-SpawnSystem: Oriented %1 to terrain", result.Prefab);
		}
		
		SCR_SlotCompositionComponent slotComp = SCR_SlotCompositionComponent.Cast(entity.FindComponent(SCR_SlotCompositionComponent));
		
		if(slotComp)
			slotComp.OrientToTerrain();
		
		SCR_AIWorld aiWorld = SCR_AIWorld.Cast(GetGame().GetAIWorld());
		if(aiWorld)
			aiWorld.RequestNavmeshRebuildEntity(entity);
		
		_placedEntities.Insert(entity);
		return true;
	}
	
	static int FindOpenAreasForCompositionType(vector centerPosition, float radius, TW_CompositionSize size, out notnull array<vector> positions, int maxResults = 20)
	{
		int positionCount = SCR_WorldTools.FindAllEmptyTerrainPositions(positions, centerPosition, radius, (int)size, 10, maxResults, TraceFlags.ENTS | TraceFlags.WORLD);
		
		EWaterSurfaceType type;
		float lakeArea;
		for(int i = 0; i < positionCount; i++)
		{
			if(i >= positions.Count()) 
				break;
			
			vector currentPos = positions.Get(i);
			
			float waterSurfaceY = SCR_WorldTools.GetWaterSurfaceY(GetGame().GetWorld(), currentPos, type, lakeArea);
			
			if(waterSurfaceY >= currentPos[1])
			{
				positions.Remove(i);
				i -= 1;
				positionCount -= 1;				
			}
		}
		
		return positionCount;
	}
	
	static bool FindOpenAreaForComposition(vector centerPosition, float radius, TW_CompositionSize size, out vector position)
	{
		bool foundPosition = SCR_WorldTools.FindEmptyTerrainPosition(position, centerPosition, radius, (int)size, (int)size, TraceFlags.ENTS | TraceFlags.WORLD);		
		
		if(!foundPosition)
			return false;
		
		EntitySpawnParams params = EntitySpawnParams();
		vector mat[4];
		mat[3] = position;
		return SCR_TerrainHelper.SnapToTerrain(mat, GetGame().GetWorld(), true);
	}		
};
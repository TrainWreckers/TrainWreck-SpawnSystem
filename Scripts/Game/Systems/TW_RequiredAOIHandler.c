class TW_RequiredAOIHandler
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
	
	private ref TW_AOIConfig _config;	
	
	const int SPAWN_DELAY = 500;
	const int PADDING = 10;
	
	private vector _previousPosition;
	private vector _areaOfInterestPosition;
	private float _areaOfInterestRadius;
	
	private int _currentAttempt;
	private int _aoiPlacementAttempt;
	private TW_AOIFailedReason _failedReason = TW_AOIFailedReason.NONE;
	
	void ~TW_RequiredAOIHandler()
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
	
	void TW_RequiredAOIHandler(TW_AOIConfig config)
	{	
		_config = config;
				
		_mapManager = GetGame().GetMapManager();
		_gameMode = SCR_BaseGameMode.Cast(GetGame().GetGameMode());	
	}
	
	private bool HasOverlap(vector search, float radius, notnull array<ref TW_CompositionPlacementResult> results)
	{
		foreach(TW_CompositionPlacementResult result : results)
		{			
			float dist = vector.Distance(search, result.Position);			
			if(dist < radius + (float)result.CompositionSize + PADDING)
				return true;
		}
		
		return false;
	}
	
	private vector RandomPos(vector position, float radius)
	{
		float angle = Math.RandomFloatInclusive(0, 359);
		float x = position[0] + radius * Math.Cos(angle);
		float y = position[2] + radius * Math.Sin(angle);
			
		return Vector(x, 0, y);
	}
	
	private void GetPlacementCircles(notnull out array<ref TW_CompositionPlacementResult> results)
	{
		float smallSize = (float)(TW_CompositionSize.SMALL);
		float mediumSize = (float)(TW_CompositionSize.MEDIUM);
		float largeSize = (float)(TW_CompositionSize.LARGE);
		
		ref array<ref TW_MustSpawnPrefabConfig> mustSpawnConfigs = _config.GetMustSpawnPrefabConfigs();
		
		foreach(TW_MustSpawnPrefabConfig spawnConfig : mustSpawnConfigs)
		{
			float size = (float)spawnConfig.GetSize();		
			float sizeRatio = (float)(size - smallSize) / (largeSize - smallSize);	
			float maxDistance = Math.Max(_areaOfInterestRadius - _config.GetMaxRadius(), _config.GetMinRadius());
			float minDistance = _areaOfInterestRadius * (1 - sizeRatio);
			
			float distanceFromCenter = Math.RandomFloatInclusive(minDistance, maxDistance);
			
			vector newPos = RandomPos(_areaOfInterestPosition, distanceFromCenter);
			
			bool overlaps = HasOverlap(newPos, size, results);
			
			int localAttempts = 75;
			while(overlaps && localAttempts > 0)
			{
				localAttempts -= 1;
				distanceFromCenter = Math.RandomFloatInclusive(minDistance, maxDistance);
				newPos = RandomPos(_areaOfInterestPosition, distanceFromCenter);
				overlaps = HasOverlap(newPos, size, results);
			}			
			
			float angle = Math.RandomFloatInclusive(0, 360);
			
			ref TW_CompositionPlacementResult result = new TW_CompositionPlacementResult();
			result.Prefab = spawnConfig.GetPrefab();
			result.Position = newPos;
			result.Angle = angle;
			result.CompositionSize = spawnConfig.GetSize();
			results.Insert(result);
		}		
	}
	
	private bool GetStartingArea(out vector position, out float radius)
	{
		EMapDescriptorType mapConfigType = _config.GetSpawnableAreas().GetRandomElement();
		ref LocationConfig locationConfig = mapManager.settings.GetConfigType(mapConfigType);
		
		int locationChunkRadius = 1;
		if(locationConfig)
			locationChunkRadius = locationConfig.radius;
		
		vector locationPosition;
		mapManager.GetRandomPositionByMapType(mapConfigType, locationPosition);		
		float searchRadius = locationChunkRadius * mapManager.settings.gridSize * 2;
		position = TW_Util.RandomPositionAround(locationPosition, searchRadius, _config.GetDistanceFromCenter());				
		radius = Math.RandomFloatInclusive(_config.GetMinRadius(), _config.GetMaxRadius());
		
		float multiplier = 1.25;
		for(int i = 0; i < 50; i++)
		{
			if(!TW_Util.IsWater(position))
				break;
			
			PrintFormat("TrainWreck: Attempt(%1/50) - %2 is under water", i, position);
			if(SCR_WorldTools.FindEmptyTerrainPosition(position, locationPosition, searchRadius * (i + multiplier), 25, 10, TraceFlags.ENTS))
				break;
		}
		
		return true;
	}
	
	private ref TW_MapManager mapManager;
	bool StartSpawn()
	{
		_placements.Clear();
		
		mapManager = TW_MapManager.GetInstance();
		
		if(!mapManager)
		{
			PrintFormat("TrainWreck: Failed to spawn required AOI handler. TW_MapManager is not initialized", LogLevel.ERROR);
			return false;			
		}
		
		GetStartingArea(_areaOfInterestPosition, _areaOfInterestRadius);			
						
		_previousPosition = _areaOfInterestPosition;
				
		GetPlacementCircles(_placements);
		SpawnNext();
		
		return !_placedEntities.IsEmpty();
	}
	
	private void SpawnNext()
	{
		if(_placements.IsEmpty())
			return;
		
		ref TW_CompositionPlacementResult result = _placements.Get(0);
		
		ref array<vector> positions = {};
		float multiplier = 1.1;
		float searchRadius = (float)result.CompositionSize * 1.25;
		int foundCount;
		
		for(int i = 0; i < 3; i++)
		{
			positions.Clear();
			foundCount = FindOpenAreasForCompositionType(result.Position, searchRadius * (i + multiplier), result.CompositionSize, positions);
		
			if(foundCount > 0)
			{
				result.Position = positions.GetRandomElement();
				break;
			}
		}
		
		positions.Clear();
		
		SpawnFromResult(result);		
		_placements.Remove(0);
		
		GetGame().GetCallqueue().CallLater(SpawnNext, SPAWN_DELAY, false);
	}
	
	private void Fail(TW_AOIFailedReason reason)
	{
		_failedReason = reason;
		
		if(GetOnAOIFailed())
			GetOnAOIFailed().Invoke(reason);					
	}
	
	bool SpawnFromResult(TW_CompositionPlacementResult result)
	{
		EntitySpawnParams params = EntitySpawnParams();
		vector mat[4];
		mat[3] = result.Position;
		
		params.Transform = mat;
		params.TransformMode = ETransformMode.WORLD;
		
		vector angles = Math3D.MatrixToAngles(params.Transform);
		angles[0] = result.Angle;
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
		
		SCR_AIWorld aiWorld = SCR_AIWorld.Cast(GetGame().GetAIWorld());
		if(aiWorld)
			aiWorld.RequestNavmeshRebuildEntity(entity);
		
		_placedEntities.Insert(entity);
		return true;
	}
	
	static int FindOpenAreasForCompositionType(vector centerPosition, float radius, TW_CompositionSize size, out notnull array<vector> positions, int maxResults = 20)
	{
		int positionCount = SCR_WorldTools.FindAllEmptyTerrainPositions(positions, centerPosition, radius, (int)size, 10, maxResults, TraceFlags.ENTS);
		
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
};
//! Intended for generating areas of interest
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
	
	//! Try to place the AOI within this distance of AreaCenter
	float AreaRadius;
	
	//! Minimum radius for AOI
	float MinRadius;
	
	//! Maximum radius for AOI
	float MaxRadius;
	
	//! AOI should be at least this far from area center
	int MinimumDistanceFromAreaCenter = 50;
	
	void TW_CompositionSpawnParameters(TW_FactionCompositions settings)
	{
		if(!settings)
			Debug.Error("TW_FactionCompositions cannot be null");
		
		Compositions = settings;
		NextSearchPosition = 1.25;
	}
	
	float GetAOIRadius()
	{
		return Math.RandomFloatInclusive(MinRadius, MaxRadius);
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
		params.MaxRadius = MaxRadius;
		params.MinRadius = MinRadius;
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
	
	//! Rotation to apply during placement
	float Angle;
	
	bool IsDefensive;
	
	string toString()
	{
		return string.Format("TrainWreck:\n\tPrefab: %1\n\tPos: %2\n\tAngle: %3\n\tSize: %4\n", Prefab, Position, Angle, CompositionSize);
	}
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
	private float _areaOfInterestRadius;
	
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
	
	private ref array<TW_CompositionType> compositionTypes = {};
	private TW_CompositionType currentType;
	
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
		ref array<TW_CompositionType> types = {TW_CompositionType.LARGE, TW_CompositionType.MEDIUM, TW_CompositionType.SMALL, TW_CompositionType.WALLS, TW_CompositionType.BUNKERS};
		
		float smallSize = (float)(TW_CompositionSize.SMALL);
		float mediumSize = (float)(TW_CompositionSize.MEDIUM);
		float largeSize = (float)(TW_CompositionSize.LARGE);
		
		foreach(TW_CompositionType type : types)
		{
			int count = GetCountFor(type);
			if(count == 0)
				continue;
				
			TW_CompositionSize sizeType = GetSize(type);
			float size = (float)sizeType;
				
			float sizeRatio = (float)(size - smallSize) / (largeSize - smallSize);
			float maxDistance = _areaOfInterestRadius - _parameters.MaxRadius;
			float minDistance = _areaOfInterestRadius * (1 - sizeRatio);
						
			// Random angle for placement
			float angle;			
			
			for(int i = 0; i < count; i++)
			{
				int localAttempts = 75;
				float distanceFromCenter = Math.RandomFloatInclusive(minDistance, maxDistance);	
				
				vector newPos = RandomPos(_areaOfInterestPosition, distanceFromCenter);
				
				bool overlaps = HasOverlap(newPos, size, results);
				
				while(localAttempts > 0 && overlaps)
				{
					localAttempts -= 1;
					newPos = RandomPos(_areaOfInterestPosition, distanceFromCenter);
					overlaps = HasOverlap(newPos, size, results);
				}
			
				if(overlaps)
					continue;
				
				bool isDefensive = false;
				if(currentType == TW_CompositionType.WALLS || currentType == TW_CompositionType.BUNKERS)
				{
					angle = TW_Util.GetAngleTo(_areaOfInterestPosition, newPos) + Math.RandomFloatInclusive(-45, 45);
					isDefensive = true;
				}
				else 
					angle = Math.RandomFloatInclusive(0, 359);			
				
				ref TW_CompositionPlacementResult result = new TW_CompositionPlacementResult();
				result.Prefab = GetCompositionPrefab(type);
				result.Position = newPos;
				result.CompositionSize = sizeType;
				result.Angle = angle;
				result.IsDefensive = isDefensive;
				PrintFormat("TrainWreck: Pos: %1, Angle: %2, Size: %3", newPos, result.Angle, sizeType);
				results.Insert(result);
			}
		}
	}
	
	private bool GetStartingArea(out vector position, out float radius)
	{		
		float searchRadius = _parameters.AreaRadius;
		
		position = TW_Util.RandomPositionAround(_parameters.AreaCenter, searchRadius, _parameters.MinimumDistanceFromAreaCenter);				
		radius =  _parameters.GetAOIRadius();
		
		float multiplier = 1.25;
		for(int i = 0; i < 50; i++)
		{
			if(!TW_Util.IsWater(position))
				break;
			
			PrintFormat("TrainWreck: Attempt(%1/50) - %2 is under water", i, position);
			if(SCR_WorldTools.FindEmptyTerrainPosition(position, _parameters.AreaCenter, searchRadius * (i + multiplier), 25, 10, TraceFlags.ENTS))
				break;
		}
		
		return true;
	}
	
	bool StartSpawn()
	{
		if(_aoiPlacementAttempt <= 0) 
			return false;
		
		_placements.Clear();
		_currentAttempt = _parameters.RetriesPerPlacement;		
		_aoiPlacementAttempt -= 1;
		
		GetStartingArea(_areaOfInterestPosition, _areaOfInterestRadius);		
		_previousPosition = _areaOfInterestPosition;
		
		SCR_Enum.GetEnumValues(TW_CompositionType, compositionTypes);
		currentType = compositionTypes.GetRandomElement();
				
		GetPlacementCircles(_placements);
		
		SpawnNext();
		// Next();
		
		return !_placedEntities.IsEmpty();
	}
	
	private void SpawnNext()
	{
		if(_placements.IsEmpty())
			return;
		
		ref TW_CompositionPlacementResult result = _placements.Get(0);
		
		ref array<vector> positions = {};
		
		float multiplier = 1.25;
		float searchRadius = (float)result.CompositionSize * 2.5;
		int foundCount;
		
		for(int i = 0; i < 10; i++)
		{
			positions.Clear();
			foundCount = FindOpenAreasForCompositionType(result.Position, searchRadius * (i + multiplier), result.CompositionSize, positions);
		
			if(foundCount > 0)
			{
				result.Position = positions.GetRandomElement();
				break;
			}
		}
		
		if(result.IsDefensive)
			result.Angle = TW_Util.GetAngleTo(_areaOfInterestPosition, result.Position) + Math.RandomFloatInclusive(-30, 30);
						
		positions.Clear();
		
		SpawnFromResult(result);		
		_placements.Remove(0);
		
		GetGame().GetCallqueue().CallLater(SpawnNext, SPAWN_DELAY, false);
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
	
	
	private void Next()
	{				
		if(compositionTypes.IsEmpty())
		{
			PrintFormat("TrainWreck: Exhausted area of interested handler: %1 - Placed", _placedEntities.Count());
			return;
		}
		
		// Pick something random that still needs to be spawned
		currentType = compositionTypes.GetRandomElement();
		
		int remaining = GetCountFor(currentType);
		bool canSpawn = remaining > 0;
		
		if(!canSpawn)
		{
			compositionTypes.RemoveItem(currentType);
		}				
		else
		{
			TW_CompositionSize minimum = GetSize(currentType);
			
			int radius = (minimum * _parameters.NextSearchPosition) + PADDING;
			vector placementArea = TW_Util.RandomPositionAround(_areaOfInterestPosition, radius, minimum);
			
			vector position;
			if(FindOpenAreaForComposition(placementArea, radius, (int) minimum, position))
			{
				ref TW_CompositionPlacementResult result = new TW_CompositionPlacementResult();
				result.Prefab = GetCompositionPrefab(currentType);
				result.Position = position;
				result.CompositionSize = minimum;
				
				if(currentType == TW_CompositionType.WALLS || currentType == TW_CompositionType.BUNKERS)
					result.Angle = Math.RandomFloatInclusive(0, 359); // (Math.Atan2(position[2] - averageZ, position[0] - averageX) * -180 / Math.PI) + Math.RandomFloatInclusive(-45, 45);
				else
					result.Angle = Math.RandomFloatInclusive(0, 359);
				
				SpawnFromResult(result);
			}
			else 
				PrintFormat("TrainWreck: Failed to place '%1', Radius: %2, Min: %3", SCR_Enum.GetEnumName(TW_CompositionType, currentType), radius, minimum, LogLevel.WARNING);
		}
		
		PrintFormat("TrainWreck: %1 has %2 remaining", SCR_Enum.GetEnumName(TW_CompositionType, currentType), remaining);
		Decrement(currentType);		
		GetGame().GetCallqueue().CallLater(Next, SPAWN_DELAY, false);
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
				_parameters.DefensiveBunkers -= 1;
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
		int positionCount = SCR_WorldTools.FindAllEmptyTerrainPositions(positions, centerPosition, radius, (int)size + 3, 10, maxResults, TraceFlags.ENTS);
		
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
		bool foundPosition = SCR_WorldTools.FindEmptyTerrainPosition(position, centerPosition, radius, (int)size + 3, (int)size, TraceFlags.ENTS);		
		
		if(!foundPosition)
			return false;
		
		EntitySpawnParams params = EntitySpawnParams();
		vector mat[4];
		mat[3] = position;
		return SCR_TerrainHelper.SnapToTerrain(mat, GetGame().GetWorld(), true);
	}		
};
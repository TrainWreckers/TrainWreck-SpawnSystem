modded class SCR_AIGroup
{
	protected bool m_ExcludeFromWanderingSystem;
	protected bool m_ExcludeFromGlobalCount;
	protected bool m_IsWanderer = false;
	
	bool IsWanderer() { return m_IsWanderer; }
	void SetWanderer(bool value) { m_IsWanderer = value; }
	
	void SetIgnoreGlobalCount(bool value)
	{
		m_ExcludeFromGlobalCount = value;
	}
	
	void SetIgnoreWandering(bool value)
	{
		m_ExcludeFromWanderingSystem = value;
	}
	
	bool IgnoreGlobalCount() { return m_ExcludeFromGlobalCount; }
	bool IgnoreWanderingSystem() { return m_ExcludeFromWanderingSystem; }
};
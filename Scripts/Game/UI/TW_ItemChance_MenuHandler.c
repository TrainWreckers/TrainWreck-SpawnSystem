class TW_ItemChanceInfo
{
	ResourceName Icon;
	string Name;
	
	void TW_ItemChanceInfo(string name, ResourceName icon)
	{
		Icon = icon;
		Name = name;
	}
	
	string GetName() { return Name; }
	ResourceName GetIcon() { return Icon; }
}

class TW_ItemChance_MenuHandler : SCR_ScriptedWidgetComponent
{
	const string s_ItemSlider = "ItemSlider";
	const string s_ItemName = "ItemName";
	const string s_ItemImage = "ItemImage";
	
	protected Widget m_Root;
	protected ref PrefabItemChance _item;
	
	override void HandlerAttached(Widget w)
	{
		super.HandlerAttached(w);
		
		m_Root = w;
	}
	
	private ref TW_ItemChanceInfo GetInfo()
	{
		ref SCR_EditableEntityUIInfo info = TW_Util.GetCharacterUIInfo(_item.PrefabName);
		
		ResourceName iconInfo = ResourceName.Empty;
		string name = string.Empty;
		
		if(info)
		{
			if(info.GetImage())
				iconInfo = info.GetImage();
			else if(info.GetIconPath())
				iconInfo = info.GetIconPath();
			
			name = info.GetName();
		}
		else 
		{
			ref SCR_EditableVehicleUIInfo vehicleInfo = TW_Util.GetVehicleUIInfo(_item.PrefabName);
		
			if(vehicleInfo)
			{
				if(vehicleInfo.GetImage())
					iconInfo = vehicleInfo.GetImage();
				else if(vehicleInfo.GetIconPath())
					iconInfo = vehicleInfo.GetIconPath();
				
				name = vehicleInfo.GetName();
			}	
		}
		
		
		return TW_ItemChanceInfo(name, iconInfo);
	}
	
	void LinkTo(PrefabItemChance item)
	{
		if(!Resource.Load(item.PrefabName).IsValid())
			return;
		
		_item = item;
		
		GetSlider().m_OnChanged.Insert(OnSliderChanged);
		GetSlider().SetValue(item.Chance);
	
		ref TW_ItemChanceInfo itemInfo = GetInfo();
		
		if(!itemInfo.Icon) 
		{
			GetImageWidget().SetVisible(false);
		}
		else
		{
			GetImageWidget().LoadImageTexture(0, itemInfo.Icon);
		}
		
		if(!itemInfo.Name)
		{
			ref array<string> parts = {};
			item.PrefabName.Split("/", parts, false);
			
			string last = parts[parts.Count() - 1];
			last.Replace(".et", "");
			
			GetNameWidget().SetText(last);
		}
		else
		{
			GetNameWidget().SetText(itemInfo.Name);
		}
	}
	
	private void OnSliderChanged(SCR_SliderComponent comp, float value)
	{
		_item.Chance = (int)value;
	}
	
	ImageWidget GetImageWidget()
	{
		Widget temp = m_Root.FindAnyWidget(s_ItemImage);
		if(!temp) return null;
		return ImageWidget.Cast(temp);
	}
	
	SCR_SliderComponent GetSlider()
	{
		Widget temp = m_Root.FindAnyWidget(s_ItemSlider);
		
		if(!temp) return null;
		
		return SCR_SliderComponent.Cast(temp.FindHandler(SCR_SliderComponent));
	}
	
	TextWidget GetNameWidget()
	{
		Widget temp = m_Root.FindAnyWidget(s_ItemName);
		if(!temp) return null;
		return TextWidget.Cast(temp);
	}
	
};
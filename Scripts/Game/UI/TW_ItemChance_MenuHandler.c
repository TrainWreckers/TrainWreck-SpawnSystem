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
	
	void LinkTo(PrefabItemChance item)
	{
		if(!Resource.Load(item.PrefabName).IsValid())
			return;
		
		_item = item;
		
		GetSlider().m_OnChanged.Insert(OnSliderChanged);
		GetSlider().SetValue(item.Chance);
	
		ref SCR_EditableEntityUIInfo info = TW_Util.GetCharacterUIInfo(item.PrefabName);
		
		if(!info) 
		{
			GetImageWidget().SetVisible(false);
			ref array<string> parts = {};
			item.PrefabName.Split("/", parts, false);
			
			string last = parts[parts.Count() - 1];
			last.Replace(".et", "");
			
			GetNameWidget().SetText(last);
			return;
		}
		
		if(info.GetImage())
			GetImageWidget().LoadImageTexture(0, info.GetImage());
		else if(info.GetIconPath())
			GetImageWidget().LoadImageTexture(0, info.GetIconPath());
		else GetImageWidget().SetVisible(false);
		
		GetNameWidget().SetText(info.GetName());
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
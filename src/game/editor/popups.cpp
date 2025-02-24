/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */

#include <base/color.h>

#include <engine/console.h>
#include <engine/graphics.h>
#include <engine/input.h>
#include <engine/keys.h>
#include <engine/shared/config.h>
#include <engine/storage.h>
#include <limits>

#include <game/client/ui_scrollregion.h>
#include <game/editor/mapitems/image.h>
#include <game/editor/mapitems/sound.h>

#include "editor.h"
#include "editor_actions.h"

CUI::EPopupMenuFunctionResult CEditor::PopupMenuFile(void *pContext, CUIRect View, bool Active)
{
	CEditor *pEditor = static_cast<CEditor *>(pContext);

	static int s_NewMapButton = 0;
	static int s_SaveButton = 0;
	static int s_SaveAsButton = 0;
	static int s_SaveCopyButton = 0;
	static int s_OpenButton = 0;
	static int s_OpenCurrentMapButton = 0;
	static int s_AppendButton = 0;
	static int s_MapInfoButton = 0;
	static int s_ExitButton = 0;

	CUIRect Slot;
	View.HSplitTop(12.0f, &Slot, &View);
	if(pEditor->DoButton_MenuItem(&s_NewMapButton, "New", 0, &Slot, 0, "Creates a new map (ctrl+n)"))
	{
		if(pEditor->HasUnsavedData())
		{
			pEditor->m_PopupEventType = POPEVENT_NEW;
			pEditor->m_PopupEventActivated = true;
		}
		else
		{
			pEditor->Reset();
			pEditor->m_aFileName[0] = 0;
		}
		return CUI::POPUP_CLOSE_CURRENT;
	}

	View.HSplitTop(10.0f, nullptr, &View);
	View.HSplitTop(12.0f, &Slot, &View);
	if(pEditor->DoButton_MenuItem(&s_OpenButton, "Load", 0, &Slot, 0, "Opens a map for editing (ctrl+l)"))
	{
		if(pEditor->HasUnsavedData())
		{
			pEditor->m_PopupEventType = POPEVENT_LOAD;
			pEditor->m_PopupEventActivated = true;
		}
		else
			pEditor->InvokeFileDialog(IStorage::TYPE_ALL, FILETYPE_MAP, "Load map", "Load", "maps", false, CEditor::CallbackOpenMap, pEditor);
		return CUI::POPUP_CLOSE_CURRENT;
	}

	View.HSplitTop(2.0f, nullptr, &View);
	View.HSplitTop(12.0f, &Slot, &View);
	if(pEditor->DoButton_MenuItem(&s_OpenCurrentMapButton, "Load Current Map", 0, &Slot, 0, "Opens the current in game map for editing (ctrl+alt+l)"))
	{
		if(pEditor->HasUnsavedData())
		{
			pEditor->m_PopupEventType = POPEVENT_LOADCURRENT;
			pEditor->m_PopupEventActivated = true;
		}
		else
		{
			pEditor->LoadCurrentMap();
		}
		return CUI::POPUP_CLOSE_CURRENT;
	}

	View.HSplitTop(10.0f, nullptr, &View);
	View.HSplitTop(12.0f, &Slot, &View);
	if(pEditor->DoButton_MenuItem(&s_AppendButton, "Append", 0, &Slot, 0, "Opens a map and adds everything from that map to the current one (ctrl+a)"))
	{
		pEditor->InvokeFileDialog(IStorage::TYPE_ALL, FILETYPE_MAP, "Append map", "Append", "maps", false, CEditor::CallbackAppendMap, pEditor);
		return CUI::POPUP_CLOSE_CURRENT;
	}

	View.HSplitTop(10.0f, nullptr, &View);
	View.HSplitTop(12.0f, &Slot, &View);
	if(pEditor->DoButton_MenuItem(&s_SaveButton, "Save", 0, &Slot, 0, "Saves the current map (ctrl+s)"))
	{
		if(pEditor->m_aFileName[0] && pEditor->m_ValidSaveFilename)
		{
			str_copy(pEditor->m_aFileSaveName, pEditor->m_aFileName);
			pEditor->m_PopupEventType = POPEVENT_SAVE;
			pEditor->m_PopupEventActivated = true;
		}
		else
			pEditor->InvokeFileDialog(IStorage::TYPE_SAVE, FILETYPE_MAP, "Save map", "Save", "maps", false, CEditor::CallbackSaveMap, pEditor);
		return CUI::POPUP_CLOSE_CURRENT;
	}

	View.HSplitTop(2.0f, nullptr, &View);
	View.HSplitTop(12.0f, &Slot, &View);
	if(pEditor->DoButton_MenuItem(&s_SaveAsButton, "Save As", 0, &Slot, 0, "Saves the current map under a new name (ctrl+shift+s)"))
	{
		pEditor->InvokeFileDialog(IStorage::TYPE_SAVE, FILETYPE_MAP, "Save map", "Save", "maps", true, CEditor::CallbackSaveMap, pEditor);
		return CUI::POPUP_CLOSE_CURRENT;
	}

	View.HSplitTop(2.0f, nullptr, &View);
	View.HSplitTop(12.0f, &Slot, &View);
	if(pEditor->DoButton_MenuItem(&s_SaveCopyButton, "Save Copy", 0, &Slot, 0, "Saves a copy of the current map under a new name (ctrl+shift+alt+s)"))
	{
		pEditor->InvokeFileDialog(IStorage::TYPE_SAVE, FILETYPE_MAP, "Save map", "Save", "maps", true, CEditor::CallbackSaveCopyMap, pEditor);
		return CUI::POPUP_CLOSE_CURRENT;
	}

	View.HSplitTop(10.0f, nullptr, &View);
	View.HSplitTop(12.0f, &Slot, &View);
	if(pEditor->DoButton_MenuItem(&s_MapInfoButton, "Map details", 0, &Slot, 0, "Adjust the map details of the current map"))
	{
		const CUIRect *pScreen = pEditor->UI()->Screen();
		pEditor->m_Map.m_MapInfoTmp.Copy(pEditor->m_Map.m_MapInfo);
		static SPopupMenuId s_PopupMapInfoId;
		constexpr float PopupWidth = 400.0f;
		constexpr float PopupHeight = 170.0f;
		pEditor->UI()->DoPopupMenu(&s_PopupMapInfoId, pScreen->w / 2.0f - PopupWidth / 2.0f, pScreen->h / 2.0f - PopupHeight / 2.0f, PopupWidth, PopupHeight, pEditor, PopupMapInfo);
		pEditor->UI()->SetActiveItem(nullptr);
		return CUI::POPUP_CLOSE_CURRENT;
	}

	View.HSplitTop(10.0f, nullptr, &View);
	View.HSplitTop(12.0f, &Slot, &View);
	if(pEditor->DoButton_MenuItem(&s_ExitButton, "Exit", 0, &Slot, 0, "Exits from the editor"))
	{
		if(pEditor->HasUnsavedData())
		{
			pEditor->m_PopupEventType = POPEVENT_EXIT;
			pEditor->m_PopupEventActivated = true;
		}
		else
		{
			pEditor->OnClose();
			g_Config.m_ClEditor = 0;
		}
		return CUI::POPUP_CLOSE_CURRENT;
	}

	return CUI::POPUP_KEEP_OPEN;
}

CUI::EPopupMenuFunctionResult CEditor::PopupMenuTools(void *pContext, CUIRect View, bool Active)
{
	CEditor *pEditor = static_cast<CEditor *>(pContext);

	CUIRect Slot;
	View.HSplitTop(12.0f, &Slot, &View);
	static int s_RemoveUnusedEnvelopesButton = 0;
	static CUI::SConfirmPopupContext s_ConfirmPopupContext;
	if(pEditor->DoButton_MenuItem(&s_RemoveUnusedEnvelopesButton, "Remove unused envelopes", 0, &Slot, 0, "Removes all unused envelopes from the map"))
	{
		s_ConfirmPopupContext.Reset();
		s_ConfirmPopupContext.YesNoButtons();
		str_copy(s_ConfirmPopupContext.m_aMessage, "Are you sure that you want to remove all unused envelopes from this map?");
		pEditor->UI()->ShowPopupConfirm(Slot.x + Slot.w, Slot.y, &s_ConfirmPopupContext);
	}
	if(s_ConfirmPopupContext.m_Result == CUI::SConfirmPopupContext::CONFIRMED)
		pEditor->RemoveUnusedEnvelopes();
	if(s_ConfirmPopupContext.m_Result != CUI::SConfirmPopupContext::UNSET)
	{
		s_ConfirmPopupContext.Reset();
		return CUI::POPUP_CLOSE_CURRENT;
	}

	static int s_BorderButton = 0;
	View.HSplitTop(2.0f, nullptr, &View);
	View.HSplitTop(12.0f, &Slot, &View);
	if(pEditor->DoButton_MenuItem(&s_BorderButton, "Place Border", 0, &Slot, 0, "Place tiles in a 2-tile wide border at the edges of the layer"))
	{
		std::shared_ptr<CLayerTiles> pT = std::static_pointer_cast<CLayerTiles>(pEditor->GetSelectedLayerType(0, LAYERTYPE_TILES));
		if(pT && !pT->m_Tele && !pT->m_Speedup && !pT->m_Switch && !pT->m_Front && !pT->m_Tune)
		{
			pEditor->m_PopupEventType = POPEVENT_PLACE_BORDER_TILES;
			pEditor->m_PopupEventActivated = true;
		}
		else
		{
			static CUI::SMessagePopupContext s_MessagePopupContext;
			s_MessagePopupContext.DefaultColor(pEditor->m_pTextRender);
			str_copy(s_MessagePopupContext.m_aMessage, "No tile layer selected");
			pEditor->UI()->ShowPopupMessage(Slot.x, Slot.y + Slot.h, &s_MessagePopupContext);
		}
	}

	static int s_GotoButton = 0;
	View.HSplitTop(2.0f, nullptr, &View);
	View.HSplitTop(12.0f, &Slot, &View);
	if(pEditor->DoButton_MenuItem(&s_GotoButton, "Goto XY", 0, &Slot, 0, "Go to a specified coordinate point on the map"))
	{
		static SPopupMenuId s_PopupGotoId;
		pEditor->UI()->DoPopupMenu(&s_PopupGotoId, Slot.x, Slot.y + Slot.h, 120, 52, pEditor, PopupGoto);
	}

	static int s_TileartButton = 0;
	View.HSplitTop(2.0f, nullptr, &View);
	View.HSplitTop(12.0f, &Slot, &View);
	if(pEditor->DoButton_MenuItem(&s_TileartButton, "Add tileart", 0, &Slot, 0, "Generate tileart from image"))
	{
		pEditor->InvokeFileDialog(IStorage::TYPE_ALL, FILETYPE_IMG, "Add tileart", "Open", "mapres", false, CallbackAddTileart, pEditor);
		return CUI::POPUP_CLOSE_CURRENT;
	}

	return CUI::POPUP_KEEP_OPEN;
}

static int EntitiesListdirCallback(const char *pName, int IsDir, int StorageType, void *pUser)
{
	CEditor *pEditor = (CEditor *)pUser;
	if(!IsDir && str_endswith(pName, ".png"))
	{
		std::string Name = pName;
		pEditor->m_vSelectEntitiesFiles.push_back(Name.substr(0, Name.length() - 4));
	}

	return 0;
}

CUI::EPopupMenuFunctionResult CEditor::PopupMenuSettings(void *pContext, CUIRect View, bool Active)
{
	CEditor *pEditor = static_cast<CEditor *>(pContext);

	CUIRect Slot;
	View.HSplitTop(12.0f, &Slot, &View);
	static int s_EntitiesButtonID = 0;
	char aButtonText[64];
	str_format(aButtonText, sizeof(aButtonText), "Entities: %s", pEditor->m_SelectEntitiesImage.c_str());
	if(pEditor->DoButton_MenuItem(&s_EntitiesButtonID, aButtonText, 0, &Slot, 0, "Choose game layer entities image for different gametypes"))
	{
		pEditor->m_vSelectEntitiesFiles.clear();
		pEditor->Storage()->ListDirectory(IStorage::TYPE_ALL, "editor/entities", EntitiesListdirCallback, pEditor);
		std::sort(pEditor->m_vSelectEntitiesFiles.begin(), pEditor->m_vSelectEntitiesFiles.end());

		static SPopupMenuId s_PopupEntitiesId;
		pEditor->UI()->DoPopupMenu(&s_PopupEntitiesId, Slot.x, Slot.y + Slot.h, 250, pEditor->m_vSelectEntitiesFiles.size() * 14.0f + 10.0f, pEditor, PopupEntities);
	}

	View.HSplitTop(2.0f, nullptr, &View);
	View.HSplitTop(12.0f, &Slot, &View);
	{
		Slot.VMargin(5.0f, &Slot);

		CUIRect Label, Selector;
		Slot.VSplitMid(&Label, &Selector);
		CUIRect No, Yes;
		Selector.VSplitMid(&No, &Yes);

		pEditor->UI()->DoLabel(&Label, "Brush coloring", 10.0f, TEXTALIGN_ML);
		static int s_ButtonNo = 0;
		static int s_ButtonYes = 0;
		if(pEditor->DoButton_ButtonDec(&s_ButtonNo, "No", !pEditor->m_BrushColorEnabled, &No, 0, "Disable brush coloring"))
		{
			pEditor->m_BrushColorEnabled = false;
		}
		if(pEditor->DoButton_ButtonInc(&s_ButtonYes, "Yes", pEditor->m_BrushColorEnabled, &Yes, 0, "Enable brush coloring"))
		{
			pEditor->m_BrushColorEnabled = true;
		}
	}

	View.HSplitTop(2.0f, nullptr, &View);
	View.HSplitTop(12.0f, &Slot, &View);
	{
		Slot.VMargin(5.0f, &Slot);

		CUIRect Label, Selector;
		Slot.VSplitMid(&Label, &Selector);
		CUIRect No, Yes;
		Selector.VSplitMid(&No, &Yes);

		pEditor->UI()->DoLabel(&Label, "Allow unused", 10.0f, TEXTALIGN_ML);
		if(pEditor->m_AllowPlaceUnusedTiles != -1)
		{
			static int s_ButtonNo = 0;
			static int s_ButtonYes = 0;
			if(pEditor->DoButton_ButtonDec(&s_ButtonNo, "No", !pEditor->m_AllowPlaceUnusedTiles, &No, 0, "[ctrl+u] Disallow placing unused tiles"))
			{
				pEditor->m_AllowPlaceUnusedTiles = false;
			}
			if(pEditor->DoButton_ButtonInc(&s_ButtonYes, "Yes", pEditor->m_AllowPlaceUnusedTiles, &Yes, 0, "[ctrl+u] Allow placing unused tiles"))
			{
				pEditor->m_AllowPlaceUnusedTiles = true;
			}
		}
	}

	View.HSplitTop(2.0f, nullptr, &View);
	View.HSplitTop(12.0f, &Slot, &View);
	{
		Slot.VMargin(5.0f, &Slot);

		CUIRect Label, Selector;
		Slot.VSplitMid(&Label, &Selector);
		CUIRect Off, Dec, Hex;
		Selector.VSplitLeft(Selector.w / 3.0f, &Off, &Selector);
		Selector.VSplitMid(&Dec, &Hex);

		pEditor->UI()->DoLabel(&Label, "Show Info", 10.0f, TEXTALIGN_ML);
		static int s_ButtonOff = 0;
		static int s_ButtonDec = 0;
		static int s_ButtonHex = 0;
		if(pEditor->DoButton_ButtonDec(&s_ButtonOff, "Off", pEditor->m_ShowTileInfo == SHOW_TILE_OFF, &Off, 0, "Do not show tile information"))
		{
			pEditor->m_ShowTileInfo = SHOW_TILE_OFF;
			pEditor->m_ShowEnvelopePreview = SHOWENV_NONE;
		}
		if(pEditor->DoButton_Ex(&s_ButtonDec, "Dec", pEditor->m_ShowTileInfo == SHOW_TILE_DECIMAL, &Dec, 0, "[ctrl+i] Show tile information", IGraphics::CORNER_NONE))
		{
			pEditor->m_ShowTileInfo = SHOW_TILE_DECIMAL;
			pEditor->m_ShowEnvelopePreview = SHOWENV_NONE;
		}
		if(pEditor->DoButton_ButtonInc(&s_ButtonHex, "Hex", pEditor->m_ShowTileInfo == SHOW_TILE_HEXADECIMAL, &Hex, 0, "[ctrl+shift+i] Show tile information in hexadecimal"))
		{
			pEditor->m_ShowTileInfo = SHOW_TILE_HEXADECIMAL;
			pEditor->m_ShowEnvelopePreview = SHOWENV_NONE;
		}
	}

	View.HSplitTop(2.0f, nullptr, &View);
	View.HSplitTop(12.0f, &Slot, &View);
	{
		Slot.VMargin(5.0f, &Slot);

		CUIRect Label, Selector;
		Slot.VSplitMid(&Label, &Selector);
		CUIRect No, Yes;
		Selector.VSplitMid(&No, &Yes);

		pEditor->UI()->DoLabel(&Label, "Align quads", 10.0f, TEXTALIGN_ML);

		static int s_ButtonNo = 0;
		static int s_ButtonYes = 0;
		if(pEditor->DoButton_ButtonDec(&s_ButtonNo, "No", !g_Config.m_EdAlignQuads, &No, 0, "Do not perform quad alignment to other quads/points when moving quads"))
		{
			g_Config.m_EdAlignQuads = false;
		}
		if(pEditor->DoButton_ButtonInc(&s_ButtonYes, "Yes", g_Config.m_EdAlignQuads, &Yes, 0, "Allow quad alignment to other quads/points when moving quads"))
		{
			g_Config.m_EdAlignQuads = true;
		}
	}

	View.HSplitTop(2.0f, nullptr, &View);
	View.HSplitTop(12.0f, &Slot, &View);
	{
		Slot.VMargin(5.0f, &Slot);

		CUIRect Label, Selector;
		Slot.VSplitMid(&Label, &Selector);
		CUIRect No, Yes;
		Selector.VSplitMid(&No, &Yes);

		pEditor->UI()->DoLabel(&Label, "Show quads bounds", 10.0f, TEXTALIGN_ML);

		static int s_ButtonNo = 0;
		static int s_ButtonYes = 0;
		if(pEditor->DoButton_ButtonDec(&s_ButtonNo, "No", !g_Config.m_EdShowQuadsRect, &No, 0, "Do not show quad bounds when moving quads"))
		{
			g_Config.m_EdShowQuadsRect = false;
		}
		if(pEditor->DoButton_ButtonInc(&s_ButtonYes, "Yes", g_Config.m_EdShowQuadsRect, &Yes, 0, "Show quad bounds when moving quads"))
		{
			g_Config.m_EdShowQuadsRect = true;
		}
	}

	return CUI::POPUP_KEEP_OPEN;
}

CUI::EPopupMenuFunctionResult CEditor::PopupGroup(void *pContext, CUIRect View, bool Active)
{
	CEditor *pEditor = static_cast<CEditor *>(pContext);

	// remove group button
	CUIRect Button;
	View.HSplitBottom(12.0f, &View, &Button);
	static int s_DeleteButton = 0;

	// don't allow deletion of game group
	if(pEditor->m_Map.m_pGameGroup != pEditor->GetSelectedGroup())
	{
		if(pEditor->DoButton_Editor(&s_DeleteButton, "Delete group", 0, &Button, 0, "Delete group"))
		{
			pEditor->m_EditorHistory.RecordAction(std::make_shared<CEditorActionGroup>(pEditor, pEditor->m_SelectedGroup, true));
			pEditor->m_Map.DeleteGroup(pEditor->m_SelectedGroup);
			pEditor->m_SelectedGroup = maximum(0, pEditor->m_SelectedGroup - 1);
			return CUI::POPUP_CLOSE_CURRENT;
		}
	}
	else
	{
		if(pEditor->DoButton_Editor(&s_DeleteButton, "Clean up game tiles", 0, &Button, 0, "Removes game tiles that aren't based on a layer"))
		{
			// gather all tile layers
			std::vector<std::shared_ptr<CLayerTiles>> vpLayers;
			int GameLayerIndex = -1;
			for(int LayerIndex = 0; LayerIndex < (int)pEditor->m_Map.m_pGameGroup->m_vpLayers.size(); LayerIndex++)
			{
				auto &pLayer = pEditor->m_Map.m_pGameGroup->m_vpLayers.at(LayerIndex);
				if(pLayer != pEditor->m_Map.m_pGameLayer && pLayer->m_Type == LAYERTYPE_TILES)
					vpLayers.push_back(std::static_pointer_cast<CLayerTiles>(pLayer));
				else if(pLayer == pEditor->m_Map.m_pGameLayer)
					GameLayerIndex = LayerIndex;
			}

			// search for unneeded game tiles
			std::shared_ptr<CLayerTiles> pGameLayer = pEditor->m_Map.m_pGameLayer;
			for(int y = 0; y < pGameLayer->m_Height; ++y)
			{
				for(int x = 0; x < pGameLayer->m_Width; ++x)
				{
					if(pGameLayer->m_pTiles[y * pGameLayer->m_Width + x].m_Index > static_cast<unsigned char>(TILE_NOHOOK))
						continue;

					bool Found = false;
					for(const auto &pLayer : vpLayers)
					{
						if(x < pLayer->m_Width && y < pLayer->m_Height && pLayer->m_pTiles[y * pLayer->m_Width + x].m_Index)
						{
							Found = true;
							break;
						}
					}

					CTile Tile = pGameLayer->GetTile(x, y);
					if(!Found && Tile.m_Index != TILE_AIR)
					{
						Tile.m_Index = TILE_AIR;
						pGameLayer->SetTile(x, y, Tile);
						pEditor->m_Map.OnModify();
					}
				}
			}

			if(!pGameLayer->m_TilesHistory.empty())
			{
				if(GameLayerIndex == -1)
				{
					dbg_msg("editor", "failed to record action (GameLayerIndex not found)");
				}
				else
				{
					// record undo
					pEditor->m_EditorHistory.RecordAction(std::make_shared<CEditorActionTileChanges>(pEditor, pEditor->m_SelectedGroup, GameLayerIndex, "Clean up game tiles", pGameLayer->m_TilesHistory));
				}
				pGameLayer->ClearHistory();
			}

			return CUI::POPUP_CLOSE_CURRENT;
		}
	}

	if(pEditor->GetSelectedGroup()->m_GameGroup && !pEditor->m_Map.m_pTeleLayer)
	{
		// new tele layer
		View.HSplitBottom(5.0f, &View, nullptr);
		View.HSplitBottom(12.0f, &View, &Button);
		static int s_NewTeleLayerButton = 0;
		if(pEditor->DoButton_Editor(&s_NewTeleLayerButton, "Add tele layer", 0, &Button, 0, "Creates a new tele layer"))
		{
			std::shared_ptr<CLayer> pTeleLayer = std::make_shared<CLayerTele>(pEditor, pEditor->m_Map.m_pGameLayer->m_Width, pEditor->m_Map.m_pGameLayer->m_Height);
			pEditor->m_Map.MakeTeleLayer(pTeleLayer);
			pEditor->m_Map.m_vpGroups[pEditor->m_SelectedGroup]->AddLayer(pTeleLayer);
			int LayerIndex = pEditor->m_Map.m_vpGroups[pEditor->m_SelectedGroup]->m_vpLayers.size() - 1;
			pEditor->SelectLayer(LayerIndex);
			pEditor->m_pBrush->Clear();
			pEditor->m_EditorHistory.RecordAction(std::make_shared<CEditorActionAddLayer>(pEditor, pEditor->m_SelectedGroup, LayerIndex));
			return CUI::POPUP_CLOSE_CURRENT;
		}
	}

	if(pEditor->GetSelectedGroup()->m_GameGroup && !pEditor->m_Map.m_pSpeedupLayer)
	{
		// new speedup layer
		View.HSplitBottom(5.0f, &View, nullptr);
		View.HSplitBottom(12.0f, &View, &Button);
		static int s_NewSpeedupLayerButton = 0;
		if(pEditor->DoButton_Editor(&s_NewSpeedupLayerButton, "Add speedup layer", 0, &Button, 0, "Creates a new speedup layer"))
		{
			std::shared_ptr<CLayer> pSpeedupLayer = std::make_shared<CLayerSpeedup>(pEditor, pEditor->m_Map.m_pGameLayer->m_Width, pEditor->m_Map.m_pGameLayer->m_Height);
			pEditor->m_Map.MakeSpeedupLayer(pSpeedupLayer);
			pEditor->m_Map.m_vpGroups[pEditor->m_SelectedGroup]->AddLayer(pSpeedupLayer);
			int LayerIndex = pEditor->m_Map.m_vpGroups[pEditor->m_SelectedGroup]->m_vpLayers.size() - 1;
			pEditor->SelectLayer(LayerIndex);
			pEditor->m_pBrush->Clear();
			pEditor->m_EditorHistory.RecordAction(std::make_shared<CEditorActionAddLayer>(pEditor, pEditor->m_SelectedGroup, LayerIndex));
			return CUI::POPUP_CLOSE_CURRENT;
		}
	}

	if(pEditor->GetSelectedGroup()->m_GameGroup && !pEditor->m_Map.m_pTuneLayer)
	{
		// new tune layer
		View.HSplitBottom(5.0f, &View, nullptr);
		View.HSplitBottom(12.0f, &View, &Button);
		static int s_NewTuneLayerButton = 0;
		if(pEditor->DoButton_Editor(&s_NewTuneLayerButton, "Add tune layer", 0, &Button, 0, "Creates a new tuning layer"))
		{
			std::shared_ptr<CLayer> pTuneLayer = std::make_shared<CLayerTune>(pEditor, pEditor->m_Map.m_pGameLayer->m_Width, pEditor->m_Map.m_pGameLayer->m_Height);
			pEditor->m_Map.MakeTuneLayer(pTuneLayer);
			pEditor->m_Map.m_vpGroups[pEditor->m_SelectedGroup]->AddLayer(pTuneLayer);
			int LayerIndex = pEditor->m_Map.m_vpGroups[pEditor->m_SelectedGroup]->m_vpLayers.size() - 1;
			pEditor->SelectLayer(LayerIndex);
			pEditor->m_pBrush->Clear();
			pEditor->m_EditorHistory.RecordAction(std::make_shared<CEditorActionAddLayer>(pEditor, pEditor->m_SelectedGroup, LayerIndex));
			return CUI::POPUP_CLOSE_CURRENT;
		}
	}

	if(pEditor->GetSelectedGroup()->m_GameGroup && !pEditor->m_Map.m_pFrontLayer)
	{
		// new front layer
		View.HSplitBottom(5.0f, &View, nullptr);
		View.HSplitBottom(12.0f, &View, &Button);
		static int s_NewFrontLayerButton = 0;
		if(pEditor->DoButton_Editor(&s_NewFrontLayerButton, "Add front layer", 0, &Button, 0, "Creates a new item layer"))
		{
			std::shared_ptr<CLayer> pFrontLayer = std::make_shared<CLayerFront>(pEditor, pEditor->m_Map.m_pGameLayer->m_Width, pEditor->m_Map.m_pGameLayer->m_Height);
			pEditor->m_Map.MakeFrontLayer(pFrontLayer);
			pEditor->m_Map.m_vpGroups[pEditor->m_SelectedGroup]->AddLayer(pFrontLayer);
			int LayerIndex = pEditor->m_Map.m_vpGroups[pEditor->m_SelectedGroup]->m_vpLayers.size() - 1;
			pEditor->SelectLayer(LayerIndex);
			pEditor->m_pBrush->Clear();
			pEditor->m_EditorHistory.RecordAction(std::make_shared<CEditorActionAddLayer>(pEditor, pEditor->m_SelectedGroup, LayerIndex));
			return CUI::POPUP_CLOSE_CURRENT;
		}
	}

	if(pEditor->GetSelectedGroup()->m_GameGroup && !pEditor->m_Map.m_pSwitchLayer)
	{
		// new Switch layer
		View.HSplitBottom(5.0f, &View, nullptr);
		View.HSplitBottom(12.0f, &View, &Button);
		static int s_NewSwitchLayerButton = 0;
		if(pEditor->DoButton_Editor(&s_NewSwitchLayerButton, "Add switch layer", 0, &Button, 0, "Creates a new switch layer"))
		{
			std::shared_ptr<CLayer> pSwitchLayer = std::make_shared<CLayerSwitch>(pEditor, pEditor->m_Map.m_pGameLayer->m_Width, pEditor->m_Map.m_pGameLayer->m_Height);
			pEditor->m_Map.MakeSwitchLayer(pSwitchLayer);
			pEditor->m_Map.m_vpGroups[pEditor->m_SelectedGroup]->AddLayer(pSwitchLayer);
			int LayerIndex = pEditor->m_Map.m_vpGroups[pEditor->m_SelectedGroup]->m_vpLayers.size() - 1;
			pEditor->SelectLayer(LayerIndex);
			pEditor->m_pBrush->Clear();
			pEditor->m_EditorHistory.RecordAction(std::make_shared<CEditorActionAddLayer>(pEditor, pEditor->m_SelectedGroup, LayerIndex));
			return CUI::POPUP_CLOSE_CURRENT;
		}
	}

	// new quad layer
	View.HSplitBottom(5.0f, &View, nullptr);
	View.HSplitBottom(12.0f, &View, &Button);
	static int s_NewQuadLayerButton = 0;
	if(pEditor->DoButton_Editor(&s_NewQuadLayerButton, "Add quads layer", 0, &Button, 0, "Creates a new quad layer"))
	{
		std::shared_ptr<CLayer> pQuadLayer = std::make_shared<CLayerQuads>(pEditor);
		pEditor->m_Map.m_vpGroups[pEditor->m_SelectedGroup]->AddLayer(pQuadLayer);
		int LayerIndex = pEditor->m_Map.m_vpGroups[pEditor->m_SelectedGroup]->m_vpLayers.size() - 1;
		pEditor->SelectLayer(LayerIndex);
		pEditor->m_Map.m_vpGroups[pEditor->m_SelectedGroup]->m_Collapse = false;
		pEditor->m_EditorHistory.RecordAction(std::make_shared<CEditorActionAddLayer>(pEditor, pEditor->m_SelectedGroup, LayerIndex));
		return CUI::POPUP_CLOSE_CURRENT;
	}

	// new tile layer
	View.HSplitBottom(5.0f, &View, nullptr);
	View.HSplitBottom(12.0f, &View, &Button);
	static int s_NewTileLayerButton = 0;
	if(pEditor->DoButton_Editor(&s_NewTileLayerButton, "Add tile layer", 0, &Button, 0, "Creates a new tile layer"))
	{
		std::shared_ptr<CLayer> pTileLayer = std::make_shared<CLayerTiles>(pEditor, pEditor->m_Map.m_pGameLayer->m_Width, pEditor->m_Map.m_pGameLayer->m_Height);
		pTileLayer->m_pEditor = pEditor;
		pEditor->m_Map.m_vpGroups[pEditor->m_SelectedGroup]->AddLayer(pTileLayer);
		int LayerIndex = pEditor->m_Map.m_vpGroups[pEditor->m_SelectedGroup]->m_vpLayers.size() - 1;
		pEditor->SelectLayer(LayerIndex);
		pEditor->m_Map.m_vpGroups[pEditor->m_SelectedGroup]->m_Collapse = false;
		pEditor->m_EditorHistory.RecordAction(std::make_shared<CEditorActionAddLayer>(pEditor, pEditor->m_SelectedGroup, LayerIndex));
		return CUI::POPUP_CLOSE_CURRENT;
	}

	// new sound layer
	View.HSplitBottom(5.0f, &View, nullptr);
	View.HSplitBottom(12.0f, &View, &Button);
	static int s_NewSoundLayerButton = 0;
	if(pEditor->DoButton_Editor(&s_NewSoundLayerButton, "Add sound layer", 0, &Button, 0, "Creates a new sound layer"))
	{
		std::shared_ptr<CLayer> pSoundLayer = std::make_shared<CLayerSounds>(pEditor);
		pEditor->m_Map.m_vpGroups[pEditor->m_SelectedGroup]->AddLayer(pSoundLayer);
		int LayerIndex = pEditor->m_Map.m_vpGroups[pEditor->m_SelectedGroup]->m_vpLayers.size() - 1;
		pEditor->SelectLayer(LayerIndex);
		pEditor->m_Map.m_vpGroups[pEditor->m_SelectedGroup]->m_Collapse = false;
		pEditor->m_EditorHistory.RecordAction(std::make_shared<CEditorActionAddLayer>(pEditor, pEditor->m_SelectedGroup, LayerIndex));
		return CUI::POPUP_CLOSE_CURRENT;
	}

	// group name
	if(!pEditor->GetSelectedGroup()->m_GameGroup)
	{
		View.HSplitBottom(5.0f, &View, nullptr);
		View.HSplitBottom(12.0f, &View, &Button);
		pEditor->UI()->DoLabel(&Button, "Name:", 10.0f, TEXTALIGN_ML);
		Button.VSplitLeft(40.0f, nullptr, &Button);
		static CLineInput s_NameInput;
		s_NameInput.SetBuffer(pEditor->m_Map.m_vpGroups[pEditor->m_SelectedGroup]->m_aName, sizeof(pEditor->m_Map.m_vpGroups[pEditor->m_SelectedGroup]->m_aName));
		if(pEditor->DoEditBox(&s_NameInput, &Button, 10.0f))
			pEditor->m_Map.OnModify();
	}

	CProperty aProps[] = {
		{"Order", pEditor->m_SelectedGroup, PROPTYPE_INT, 0, (int)pEditor->m_Map.m_vpGroups.size() - 1},
		{"Pos X", -pEditor->m_Map.m_vpGroups[pEditor->m_SelectedGroup]->m_OffsetX, PROPTYPE_INT, -1000000, 1000000},
		{"Pos Y", -pEditor->m_Map.m_vpGroups[pEditor->m_SelectedGroup]->m_OffsetY, PROPTYPE_INT, -1000000, 1000000},
		{"Para X", pEditor->m_Map.m_vpGroups[pEditor->m_SelectedGroup]->m_ParallaxX, PROPTYPE_INT, -1000000, 1000000},
		{"Para Y", pEditor->m_Map.m_vpGroups[pEditor->m_SelectedGroup]->m_ParallaxY, PROPTYPE_INT, -1000000, 1000000},
		{"Use Clipping", pEditor->m_Map.m_vpGroups[pEditor->m_SelectedGroup]->m_UseClipping, PROPTYPE_BOOL, 0, 1},
		{"Clip X", pEditor->m_Map.m_vpGroups[pEditor->m_SelectedGroup]->m_ClipX, PROPTYPE_INT, -1000000, 1000000},
		{"Clip Y", pEditor->m_Map.m_vpGroups[pEditor->m_SelectedGroup]->m_ClipY, PROPTYPE_INT, -1000000, 1000000},
		{"Clip W", pEditor->m_Map.m_vpGroups[pEditor->m_SelectedGroup]->m_ClipW, PROPTYPE_INT, 0, 1000000},
		{"Clip H", pEditor->m_Map.m_vpGroups[pEditor->m_SelectedGroup]->m_ClipH, PROPTYPE_INT, 0, 1000000},
		{nullptr},
	};

	// cut the properties that aren't needed
	if(pEditor->GetSelectedGroup()->m_GameGroup)
		aProps[(int)EGroupProp::PROP_POS_X].m_pName = nullptr;

	static int s_aIds[(int)EGroupProp::NUM_PROPS] = {0};
	int NewVal = 0;
	auto [State, Prop] = pEditor->DoPropertiesWithState<EGroupProp>(&View, aProps, s_aIds, &NewVal);
	if(Prop != EGroupProp::PROP_NONE)
	{
		pEditor->m_Map.OnModify();
	}

	static CLayerGroupPropTracker s_Tracker(pEditor);
	s_Tracker.Begin(pEditor->GetSelectedGroup().get(), Prop, State);

	if(Prop == EGroupProp::PROP_ORDER)
	{
		pEditor->m_SelectedGroup = pEditor->m_Map.SwapGroups(pEditor->m_SelectedGroup, NewVal);
	}

	// these can not be changed on the game group
	if(!pEditor->GetSelectedGroup()->m_GameGroup)
	{
		if(Prop == EGroupProp::PROP_PARA_X)
		{
			pEditor->m_Map.m_vpGroups[pEditor->m_SelectedGroup]->m_ParallaxX = NewVal;
		}
		else if(Prop == EGroupProp::PROP_PARA_Y)
		{
			pEditor->m_Map.m_vpGroups[pEditor->m_SelectedGroup]->m_ParallaxY = NewVal;
		}
		else if(Prop == EGroupProp::PROP_POS_X)
		{
			pEditor->m_Map.m_vpGroups[pEditor->m_SelectedGroup]->m_OffsetX = -NewVal;
		}
		else if(Prop == EGroupProp::PROP_POS_Y)
		{
			pEditor->m_Map.m_vpGroups[pEditor->m_SelectedGroup]->m_OffsetY = -NewVal;
		}
		else if(Prop == EGroupProp::PROP_USE_CLIPPING)
		{
			pEditor->m_Map.m_vpGroups[pEditor->m_SelectedGroup]->m_UseClipping = NewVal;
		}
		else if(Prop == EGroupProp::PROP_CLIP_X)
		{
			pEditor->m_Map.m_vpGroups[pEditor->m_SelectedGroup]->m_ClipX = NewVal;
		}
		else if(Prop == EGroupProp::PROP_CLIP_Y)
		{
			pEditor->m_Map.m_vpGroups[pEditor->m_SelectedGroup]->m_ClipY = NewVal;
		}
		else if(Prop == EGroupProp::PROP_CLIP_W)
		{
			pEditor->m_Map.m_vpGroups[pEditor->m_SelectedGroup]->m_ClipW = NewVal;
		}
		else if(Prop == EGroupProp::PROP_CLIP_H)
		{
			pEditor->m_Map.m_vpGroups[pEditor->m_SelectedGroup]->m_ClipH = NewVal;
		}
	}

	s_Tracker.End(Prop, State);

	return CUI::POPUP_KEEP_OPEN;
}

CUI::EPopupMenuFunctionResult CEditor::PopupLayer(void *pContext, CUIRect View, bool Active)
{
	SLayerPopupContext *pPopup = (SLayerPopupContext *)pContext;
	CEditor *pEditor = pPopup->m_pEditor;

	std::shared_ptr<CLayerGroup> pCurrentGroup = pEditor->m_Map.m_vpGroups[pEditor->m_SelectedGroup];
	std::shared_ptr<CLayer> pCurrentLayer = pEditor->GetSelectedLayer(0);

	if(pPopup->m_vpLayers.size() > 1)
	{
		return CLayerTiles::RenderCommonProperties(pPopup->m_CommonPropState, pEditor, &View, pPopup->m_vpLayers, pPopup->m_vLayerIndices);
	}

	const bool EntitiesLayer = pCurrentLayer->IsEntitiesLayer();

	// delete button
	if(pEditor->m_Map.m_pGameLayer != pCurrentLayer) // entities layers except the game layer can be deleted
	{
		CUIRect DeleteButton;
		View.HSplitBottom(12.0f, &View, &DeleteButton);
		static int s_DeleteButton = 0;
		if(pEditor->DoButton_Editor(&s_DeleteButton, "Delete layer", 0, &DeleteButton, 0, "Deletes the layer"))
		{
			pEditor->m_EditorHistory.RecordAction(std::make_shared<CEditorActionDeleteLayer>(pEditor, pEditor->m_SelectedGroup, pEditor->m_vSelectedLayers[0]));

			if(pCurrentLayer == pEditor->m_Map.m_pFrontLayer)
				pEditor->m_Map.m_pFrontLayer = nullptr;
			if(pCurrentLayer == pEditor->m_Map.m_pTeleLayer)
				pEditor->m_Map.m_pTeleLayer = nullptr;
			if(pCurrentLayer == pEditor->m_Map.m_pSpeedupLayer)
				pEditor->m_Map.m_pSpeedupLayer = nullptr;
			if(pCurrentLayer == pEditor->m_Map.m_pSwitchLayer)
				pEditor->m_Map.m_pSwitchLayer = nullptr;
			if(pCurrentLayer == pEditor->m_Map.m_pTuneLayer)
				pEditor->m_Map.m_pTuneLayer = nullptr;
			pEditor->m_Map.m_vpGroups[pEditor->m_SelectedGroup]->DeleteLayer(pEditor->m_vSelectedLayers[0]);

			return CUI::POPUP_CLOSE_CURRENT;
		}
	}

	// duplicate button
	if(!EntitiesLayer) // entities layers cannot be duplicated
	{
		CUIRect DuplicateButton;
		View.HSplitBottom(4.0f, &View, nullptr);
		View.HSplitBottom(12.0f, &View, &DuplicateButton);
		static int s_DuplicationButton = 0;
		if(pEditor->DoButton_Editor(&s_DuplicationButton, "Duplicate layer", 0, &DuplicateButton, 0, "Duplicates the layer"))
		{
			pEditor->m_Map.m_vpGroups[pEditor->m_SelectedGroup]->DuplicateLayer(pEditor->m_vSelectedLayers[0]);
			pEditor->m_EditorHistory.RecordAction(std::make_shared<CEditorActionAddLayer>(pEditor, pEditor->m_SelectedGroup, pEditor->m_vSelectedLayers[0] + 1, true));
			return CUI::POPUP_CLOSE_CURRENT;
		}
	}

	// layer name
	if(!EntitiesLayer) // name cannot be changed for entities layers
	{
		CUIRect Label, EditBox;
		View.HSplitBottom(5.0f, &View, nullptr);
		View.HSplitBottom(12.0f, &View, &Label);
		Label.VSplitLeft(40.0f, &Label, &EditBox);
		pEditor->UI()->DoLabel(&Label, "Name:", 10.0f, TEXTALIGN_ML);
		static CLineInput s_NameInput;
		s_NameInput.SetBuffer(pCurrentLayer->m_aName, sizeof(pCurrentLayer->m_aName));
		if(pEditor->DoEditBox(&s_NameInput, &EditBox, 10.0f))
			pEditor->m_Map.OnModify();
	}

	// spacing if any button was rendered
	if(!EntitiesLayer || pEditor->m_Map.m_pGameLayer != pCurrentLayer)
		View.HSplitBottom(10.0f, &View, nullptr);

	CProperty aProps[] = {
		{"Group", pEditor->m_SelectedGroup, PROPTYPE_INT, 0, (int)pEditor->m_Map.m_vpGroups.size() - 1},
		{"Order", pEditor->m_vSelectedLayers[0], PROPTYPE_INT, 0, (int)pCurrentGroup->m_vpLayers.size() - 1},
		{"Detail", pCurrentLayer->m_Flags & LAYERFLAG_DETAIL, PROPTYPE_BOOL, 0, 1},
		{nullptr},
	};

	// don't use Group and Detail from the selection if this is an entities layer
	if(EntitiesLayer)
	{
		aProps[0].m_Type = PROPTYPE_NULL;
		aProps[2].m_Type = PROPTYPE_NULL;
	}

	static int s_aIds[(int)ELayerProp::NUM_PROPS] = {0};
	int NewVal = 0;
	auto [State, Prop] = pEditor->DoPropertiesWithState<ELayerProp>(&View, aProps, s_aIds, &NewVal);
	if(Prop != ELayerProp::PROP_NONE)
	{
		pEditor->m_Map.OnModify();
	}

	static CLayerPropTracker s_Tracker(pEditor);
	s_Tracker.Begin(pCurrentLayer.get(), Prop, State);

	if(Prop == ELayerProp::PROP_ORDER)
	{
		int NewIndex = pCurrentGroup->SwapLayers(pEditor->m_vSelectedLayers[0], NewVal);
		pEditor->SelectLayer(NewIndex);
	}
	else if(Prop == ELayerProp::PROP_GROUP)
	{
		if(NewVal >= 0 && (size_t)NewVal < pEditor->m_Map.m_vpGroups.size() && NewVal != pEditor->m_SelectedGroup)
		{
			auto Position = std::find(pCurrentGroup->m_vpLayers.begin(), pCurrentGroup->m_vpLayers.end(), pCurrentLayer);
			if(Position != pCurrentGroup->m_vpLayers.end())
				pCurrentGroup->m_vpLayers.erase(Position);
			pEditor->m_Map.m_vpGroups[NewVal]->m_vpLayers.push_back(pCurrentLayer);
			pEditor->m_SelectedGroup = NewVal;
			pEditor->SelectLayer(pEditor->m_Map.m_vpGroups[NewVal]->m_vpLayers.size() - 1);
		}
	}
	else if(Prop == ELayerProp::PROP_HQ)
	{
		pCurrentLayer->m_Flags &= ~LAYERFLAG_DETAIL;
		if(NewVal)
			pCurrentLayer->m_Flags |= LAYERFLAG_DETAIL;
	}

	s_Tracker.End(Prop, State);

	return pCurrentLayer->RenderProperties(&View);
}

CUI::EPopupMenuFunctionResult CEditor::PopupQuad(void *pContext, CUIRect View, bool Active)
{
	CEditor *pEditor = static_cast<CEditor *>(pContext);
	std::vector<CQuad *> vpQuads = pEditor->GetSelectedQuads();
	if(pEditor->m_SelectedQuadIndex < 0 || pEditor->m_SelectedQuadIndex >= (int)vpQuads.size())
		return CUI::POPUP_CLOSE_CURRENT;
	CQuad *pCurrentQuad = vpQuads[pEditor->m_SelectedQuadIndex];
	std::shared_ptr<CLayerQuads> pLayer = std::static_pointer_cast<CLayerQuads>(pEditor->GetSelectedLayerType(0, LAYERTYPE_QUADS));

	CUIRect Button;

	// delete button
	View.HSplitBottom(12.0f, &View, &Button);
	static int s_DeleteButton = 0;
	if(pEditor->DoButton_Editor(&s_DeleteButton, "Delete", 0, &Button, 0, "Deletes the current quad"))
	{
		if(pLayer)
		{
			pEditor->m_Map.OnModify();
			pEditor->DeleteSelectedQuads();
		}
		return CUI::POPUP_CLOSE_CURRENT;
	}

	// aspect ratio button
	View.HSplitBottom(10.0f, &View, nullptr);
	View.HSplitBottom(12.0f, &View, &Button);
	if(pLayer && pLayer->m_Image >= 0 && (size_t)pLayer->m_Image < pEditor->m_Map.m_vpImages.size())
	{
		static int s_AspectRatioButton = 0;
		if(pEditor->DoButton_Editor(&s_AspectRatioButton, "Aspect ratio", 0, &Button, 0, "Resizes the current Quad based on the aspect ratio of the image"))
		{
			pEditor->m_QuadTracker.BeginQuadTrack(pLayer, pEditor->m_vSelectedQuads);
			for(auto &pQuad : vpQuads)
			{
				int Top = pQuad->m_aPoints[0].y;
				int Left = pQuad->m_aPoints[0].x;
				int Right = pQuad->m_aPoints[0].x;

				for(int k = 1; k < 4; k++)
				{
					if(pQuad->m_aPoints[k].y < Top)
						Top = pQuad->m_aPoints[k].y;
					if(pQuad->m_aPoints[k].x < Left)
						Left = pQuad->m_aPoints[k].x;
					if(pQuad->m_aPoints[k].x > Right)
						Right = pQuad->m_aPoints[k].x;
				}

				const int Height = (Right - Left) * pEditor->m_Map.m_vpImages[pLayer->m_Image]->m_Height / pEditor->m_Map.m_vpImages[pLayer->m_Image]->m_Width;

				pQuad->m_aPoints[0].x = Left;
				pQuad->m_aPoints[0].y = Top;
				pQuad->m_aPoints[1].x = Right;
				pQuad->m_aPoints[1].y = Top;
				pQuad->m_aPoints[2].x = Left;
				pQuad->m_aPoints[2].y = Top + Height;
				pQuad->m_aPoints[3].x = Right;
				pQuad->m_aPoints[3].y = Top + Height;
				pEditor->m_Map.OnModify();
			}
			pEditor->m_QuadTracker.EndQuadTrack();

			return CUI::POPUP_CLOSE_CURRENT;
		}
	}

	// align button
	View.HSplitBottom(6.0f, &View, nullptr);
	View.HSplitBottom(12.0f, &View, &Button);
	static int s_AlignButton = 0;
	if(pEditor->DoButton_Editor(&s_AlignButton, "Align", 0, &Button, 0, "Aligns coordinates of the quad points"))
	{
		pEditor->m_QuadTracker.BeginQuadTrack(pLayer, pEditor->m_vSelectedQuads);
		for(auto &pQuad : vpQuads)
		{
			for(int k = 1; k < 4; k++)
			{
				pQuad->m_aPoints[k].x = 1000.0f * (pQuad->m_aPoints[k].x / 1000);
				pQuad->m_aPoints[k].y = 1000.0f * (pQuad->m_aPoints[k].y / 1000);
			}
			pEditor->m_Map.OnModify();
		}
		pEditor->m_QuadTracker.EndQuadTrack();
		return CUI::POPUP_CLOSE_CURRENT;
	}

	// square button
	View.HSplitBottom(6.0f, &View, nullptr);
	View.HSplitBottom(12.0f, &View, &Button);
	static int s_Button = 0;
	if(pEditor->DoButton_Editor(&s_Button, "Square", 0, &Button, 0, "Squares the current quad"))
	{
		pEditor->m_QuadTracker.BeginQuadTrack(pLayer, pEditor->m_vSelectedQuads);
		for(auto &pQuad : vpQuads)
		{
			int Top = pQuad->m_aPoints[0].y;
			int Left = pQuad->m_aPoints[0].x;
			int Bottom = pQuad->m_aPoints[0].y;
			int Right = pQuad->m_aPoints[0].x;

			for(int k = 1; k < 4; k++)
			{
				if(pQuad->m_aPoints[k].y < Top)
					Top = pQuad->m_aPoints[k].y;
				if(pQuad->m_aPoints[k].x < Left)
					Left = pQuad->m_aPoints[k].x;
				if(pQuad->m_aPoints[k].y > Bottom)
					Bottom = pQuad->m_aPoints[k].y;
				if(pQuad->m_aPoints[k].x > Right)
					Right = pQuad->m_aPoints[k].x;
			}

			pQuad->m_aPoints[0].x = Left;
			pQuad->m_aPoints[0].y = Top;
			pQuad->m_aPoints[1].x = Right;
			pQuad->m_aPoints[1].y = Top;
			pQuad->m_aPoints[2].x = Left;
			pQuad->m_aPoints[2].y = Bottom;
			pQuad->m_aPoints[3].x = Right;
			pQuad->m_aPoints[3].y = Bottom;
			pEditor->m_Map.OnModify();
		}
		pEditor->m_QuadTracker.EndQuadTrack();
		return CUI::POPUP_CLOSE_CURRENT;
	}

	// slice button
	View.HSplitBottom(6.0f, &View, nullptr);
	View.HSplitBottom(12.0f, &View, &Button);
	static int s_SliceButton = 0;
	if(pEditor->DoButton_Editor(&s_SliceButton, "Slice", 0, &Button, 0, "Enables quad knife mode"))
	{
		pEditor->m_QuadKnifeCount = 0;
		pEditor->m_QuadKnifeActive = true;
		return CUI::POPUP_CLOSE_CURRENT;
	}

	const int NumQuads = pLayer ? (int)pLayer->m_vQuads.size() : 0;
	CProperty aProps[] = {
		{"Order", pEditor->m_vSelectedQuads[pEditor->m_SelectedQuadIndex], PROPTYPE_INT, 0, NumQuads},
		{"Pos X", fx2i(pCurrentQuad->m_aPoints[4].x), PROPTYPE_INT, -1000000, 1000000},
		{"Pos Y", fx2i(pCurrentQuad->m_aPoints[4].y), PROPTYPE_INT, -1000000, 1000000},
		{"Pos. Env", pCurrentQuad->m_PosEnv + 1, PROPTYPE_ENVELOPE, 0, 0},
		{"Pos. TO", pCurrentQuad->m_PosEnvOffset, PROPTYPE_INT, -1000000, 1000000},
		{"Color Env", pCurrentQuad->m_ColorEnv + 1, PROPTYPE_ENVELOPE, 0, 0},
		{"Color TO", pCurrentQuad->m_ColorEnvOffset, PROPTYPE_INT, -1000000, 1000000},
		{nullptr},
	};

	static int s_aIds[(int)EQuadProp::NUM_PROPS] = {0};
	int NewVal = 0;
	auto PropRes = pEditor->DoPropertiesWithState<EQuadProp>(&View, aProps, s_aIds, &NewVal);
	EQuadProp Prop = PropRes.m_Value;
	if(Prop != EQuadProp::PROP_NONE)
	{
		pEditor->m_Map.OnModify();
		if(PropRes.m_State == EEditState::START || PropRes.m_State == EEditState::ONE_GO)
		{
			pEditor->m_QuadTracker.BeginQuadPropTrack(pLayer, pEditor->m_vSelectedQuads, Prop);
		}
	}

	const float OffsetX = i2fx(NewVal) - pCurrentQuad->m_aPoints[4].x;
	const float OffsetY = i2fx(NewVal) - pCurrentQuad->m_aPoints[4].y;

	if(Prop == EQuadProp::PROP_ORDER && pLayer)
	{
		const int QuadIndex = pLayer->SwapQuads(pEditor->m_vSelectedQuads[pEditor->m_SelectedQuadIndex], NewVal);
		pEditor->m_vSelectedQuads[pEditor->m_SelectedQuadIndex] = QuadIndex;
	}

	for(auto &pQuad : vpQuads)
	{
		if(Prop == EQuadProp::PROP_POS_X)
		{
			for(auto &Point : pQuad->m_aPoints)
				Point.x += OffsetX;
		}
		else if(Prop == EQuadProp::PROP_POS_Y)
		{
			for(auto &Point : pQuad->m_aPoints)
				Point.y += OffsetY;
		}
		else if(Prop == EQuadProp::PROP_POS_ENV)
		{
			int Index = clamp(NewVal - 1, -1, (int)pEditor->m_Map.m_vpEnvelopes.size() - 1);
			int StepDirection = Index < pQuad->m_PosEnv ? -1 : 1;
			if(StepDirection != 0)
			{
				for(; Index >= -1 && Index < (int)pEditor->m_Map.m_vpEnvelopes.size(); Index += StepDirection)
				{
					if(Index == -1 || pEditor->m_Map.m_vpEnvelopes[Index]->GetChannels() == 3)
					{
						pQuad->m_PosEnv = Index;
						break;
					}
				}
			}
		}
		else if(Prop == EQuadProp::PROP_POS_ENV_OFFSET)
		{
			pQuad->m_PosEnvOffset = NewVal;
		}
		else if(Prop == EQuadProp::PROP_COLOR_ENV)
		{
			int Index = clamp(NewVal - 1, -1, (int)pEditor->m_Map.m_vpEnvelopes.size() - 1);
			int StepDirection = Index < pQuad->m_ColorEnv ? -1 : 1;
			if(StepDirection != 0)
			{
				for(; Index >= -1 && Index < (int)pEditor->m_Map.m_vpEnvelopes.size(); Index += StepDirection)
				{
					if(Index == -1 || pEditor->m_Map.m_vpEnvelopes[Index]->GetChannels() == 4)
					{
						pQuad->m_ColorEnv = Index;
						break;
					}
				}
			}
		}
		else if(Prop == EQuadProp::PROP_COLOR_ENV_OFFSET)
		{
			pQuad->m_ColorEnvOffset = NewVal;
		}
	}

	if(Prop != EQuadProp::PROP_NONE)
	{
		if(PropRes.m_State == EEditState::END || PropRes.m_State == EEditState::ONE_GO)
		{
			pEditor->m_QuadTracker.EndQuadPropTrack(Prop);
		}
	}

	return CUI::POPUP_KEEP_OPEN;
}

CUI::EPopupMenuFunctionResult CEditor::PopupSource(void *pContext, CUIRect View, bool Active)
{
	CEditor *pEditor = static_cast<CEditor *>(pContext);
	CSoundSource *pSource = pEditor->GetSelectedSource();
	if(!pSource)
		return CUI::POPUP_CLOSE_CURRENT;

	CUIRect Button;

	// delete button
	View.HSplitBottom(12.0f, &View, &Button);
	static int s_DeleteButton = 0;
	if(pEditor->DoButton_Editor(&s_DeleteButton, "Delete", 0, &Button, 0, "Deletes the current source"))
	{
		std::shared_ptr<CLayerSounds> pLayer = std::static_pointer_cast<CLayerSounds>(pEditor->GetSelectedLayerType(0, LAYERTYPE_SOUNDS));
		if(pLayer)
		{
			pEditor->m_EditorHistory.Execute(std::make_shared<CEditorActionDeleteSoundSource>(pEditor, pEditor->m_SelectedGroup, pEditor->m_vSelectedLayers[0], pEditor->m_SelectedSource));
		}
		return CUI::POPUP_CLOSE_CURRENT;
	}

	// Sound shape button
	CUIRect ShapeButton;
	View.HSplitBottom(3.0f, &View, nullptr);
	View.HSplitBottom(12.0f, &View, &ShapeButton);

	static const char *s_apShapeNames[CSoundShape::NUM_SHAPES] = {
		"Rectangle",
		"Circle"};

	pSource->m_Shape.m_Type = pSource->m_Shape.m_Type % CSoundShape::NUM_SHAPES; // prevent out of array errors

	static int s_ShapeTypeButton = 0;
	if(pEditor->DoButton_Editor(&s_ShapeTypeButton, s_apShapeNames[pSource->m_Shape.m_Type], 0, &ShapeButton, 0, "Change shape"))
	{
		pEditor->m_EditorHistory.Execute(std::make_shared<CEditorActionEditSoundSource>(pEditor, pEditor->m_SelectedGroup, pEditor->m_vSelectedLayers[0], pEditor->m_SelectedSource, CEditorActionEditSoundSource::EEditType::SHAPE, (pSource->m_Shape.m_Type + 1) % CSoundShape::NUM_SHAPES));
	}

	CProperty aProps[] = {
		{"Pos X", pSource->m_Position.x / 1000, PROPTYPE_INT, -1000000, 1000000},
		{"Pos Y", pSource->m_Position.y / 1000, PROPTYPE_INT, -1000000, 1000000},
		{"Loop", pSource->m_Loop, PROPTYPE_BOOL, 0, 1},
		{"Pan", pSource->m_Pan, PROPTYPE_BOOL, 0, 1},
		{"Delay", pSource->m_TimeDelay, PROPTYPE_INT, 0, 1000000},
		{"Falloff", pSource->m_Falloff, PROPTYPE_INT, 0, 255},
		{"Pos. Env", pSource->m_PosEnv + 1, PROPTYPE_ENVELOPE, 0, 0},
		{"Pos. TO", pSource->m_PosEnvOffset, PROPTYPE_INT, -1000000, 1000000},
		{"Sound Env", pSource->m_SoundEnv + 1, PROPTYPE_ENVELOPE, 0, 0},
		{"Sound. TO", pSource->m_SoundEnvOffset, PROPTYPE_INT, -1000000, 1000000},
		{nullptr},
	};

	static int s_aIds[(int)ESoundProp::NUM_PROPS] = {0};
	int NewVal = 0;
	auto [State, Prop] = pEditor->DoPropertiesWithState<ESoundProp>(&View, aProps, s_aIds, &NewVal);
	if(Prop != ESoundProp::PROP_NONE)
	{
		pEditor->m_Map.OnModify();
	}

	static CSoundSourcePropTracker s_Tracker(pEditor);
	s_Tracker.Begin(pSource, Prop, State);

	if(Prop == ESoundProp::PROP_POS_X)
	{
		pSource->m_Position.x = NewVal * 1000;
	}
	else if(Prop == ESoundProp::PROP_POS_Y)
	{
		pSource->m_Position.y = NewVal * 1000;
	}
	else if(Prop == ESoundProp::PROP_LOOP)
	{
		pSource->m_Loop = NewVal;
	}
	else if(Prop == ESoundProp::PROP_PAN)
	{
		pSource->m_Pan = NewVal;
	}
	else if(Prop == ESoundProp::PROP_TIME_DELAY)
	{
		pSource->m_TimeDelay = NewVal;
	}
	else if(Prop == ESoundProp::PROP_FALLOFF)
	{
		pSource->m_Falloff = NewVal;
	}
	else if(Prop == ESoundProp::PROP_POS_ENV)
	{
		int Index = clamp(NewVal - 1, -1, (int)pEditor->m_Map.m_vpEnvelopes.size() - 1);
		const int StepDirection = Index < pSource->m_PosEnv ? -1 : 1;
		for(; Index >= -1 && Index < (int)pEditor->m_Map.m_vpEnvelopes.size(); Index += StepDirection)
		{
			if(Index == -1 || pEditor->m_Map.m_vpEnvelopes[Index]->GetChannels() == 3)
			{
				pSource->m_PosEnv = Index;
				break;
			}
		}
	}
	else if(Prop == ESoundProp::PROP_POS_ENV_OFFSET)
	{
		pSource->m_PosEnvOffset = NewVal;
	}
	else if(Prop == ESoundProp::PROP_SOUND_ENV)
	{
		int Index = clamp(NewVal - 1, -1, (int)pEditor->m_Map.m_vpEnvelopes.size() - 1);
		const int StepDirection = Index < pSource->m_SoundEnv ? -1 : 1;
		for(; Index >= -1 && Index < (int)pEditor->m_Map.m_vpEnvelopes.size(); Index += StepDirection)
		{
			if(Index == -1 || pEditor->m_Map.m_vpEnvelopes[Index]->GetChannels() == 1)
			{
				pSource->m_SoundEnv = Index;
				break;
			}
		}
	}
	else if(Prop == ESoundProp::PROP_SOUND_ENV_OFFSET)
	{
		pSource->m_SoundEnvOffset = NewVal;
	}

	s_Tracker.End(Prop, State);

	// source shape properties
	switch(pSource->m_Shape.m_Type)
	{
	case CSoundShape::SHAPE_CIRCLE:
	{
		CProperty aCircleProps[] = {
			{"Radius", pSource->m_Shape.m_Circle.m_Radius, PROPTYPE_INT, 0, 1000000},
			{nullptr},
		};

		static int s_aCircleIds[(int)ECircleShapeProp::NUM_CIRCLE_PROPS] = {0};
		NewVal = 0;
		auto [LocalState, LocalProp] = pEditor->DoPropertiesWithState<ECircleShapeProp>(&View, aCircleProps, s_aCircleIds, &NewVal);
		if(LocalProp != ECircleShapeProp::PROP_NONE)
		{
			pEditor->m_Map.OnModify();
		}

		static CSoundSourceCircleShapePropTracker s_ShapeTracker(pEditor);
		s_ShapeTracker.Begin(pSource, LocalProp, LocalState);

		if(LocalProp == ECircleShapeProp::PROP_CIRCLE_RADIUS)
		{
			pSource->m_Shape.m_Circle.m_Radius = NewVal;
		}

		s_ShapeTracker.End(LocalProp, LocalState);
		break;
	}

	case CSoundShape::SHAPE_RECTANGLE:
	{
		CProperty aRectangleProps[] = {
			{"Width", pSource->m_Shape.m_Rectangle.m_Width / 1024, PROPTYPE_INT, 0, 1000000},
			{"Height", pSource->m_Shape.m_Rectangle.m_Height / 1024, PROPTYPE_INT, 0, 1000000},
			{nullptr},
		};

		static int s_aRectangleIds[(int)ERectangleShapeProp::NUM_RECTANGLE_PROPS] = {0};
		NewVal = 0;
		auto [LocalState, LocalProp] = pEditor->DoPropertiesWithState<ERectangleShapeProp>(&View, aRectangleProps, s_aRectangleIds, &NewVal);
		if(LocalProp != ERectangleShapeProp::PROP_NONE)
		{
			pEditor->m_Map.OnModify();
		}

		static CSoundSourceRectShapePropTracker s_ShapeTracker(pEditor);
		s_ShapeTracker.Begin(pSource, LocalProp, LocalState);

		if(LocalProp == ERectangleShapeProp::PROP_RECTANGLE_WIDTH)
		{
			pSource->m_Shape.m_Rectangle.m_Width = NewVal * 1024;
		}
		else if(LocalProp == ERectangleShapeProp::PROP_RECTANGLE_HEIGHT)
		{
			pSource->m_Shape.m_Rectangle.m_Height = NewVal * 1024;
		}

		s_ShapeTracker.End(LocalProp, LocalState);
		break;
	}
	}

	return CUI::POPUP_KEEP_OPEN;
}

CUI::EPopupMenuFunctionResult CEditor::PopupPoint(void *pContext, CUIRect View, bool Active)
{
	CEditor *pEditor = static_cast<CEditor *>(pContext);
	std::vector<CQuad *> vpQuads = pEditor->GetSelectedQuads();
	if(!in_range<int>(pEditor->m_SelectedQuadIndex, 0, vpQuads.size() - 1))
		return CUI::POPUP_CLOSE_CURRENT;
	CQuad *pCurrentQuad = vpQuads[pEditor->m_SelectedQuadIndex];
	std::shared_ptr<CLayerQuads> pLayer = std::static_pointer_cast<CLayerQuads>(pEditor->GetSelectedLayerType(0, LAYERTYPE_QUADS));

	int Color = PackColor(pCurrentQuad->m_aColors[pEditor->m_SelectedQuadPoint]);

	const int X = fx2i(pCurrentQuad->m_aPoints[pEditor->m_SelectedQuadPoint].x);
	const int Y = fx2i(pCurrentQuad->m_aPoints[pEditor->m_SelectedQuadPoint].y);
	const int TextureU = fx2f(pCurrentQuad->m_aTexcoords[pEditor->m_SelectedQuadPoint].x) * 1024;
	const int TextureV = fx2f(pCurrentQuad->m_aTexcoords[pEditor->m_SelectedQuadPoint].y) * 1024;

	CProperty aProps[] = {
		{"Pos X", X, PROPTYPE_INT, -1000000, 1000000},
		{"Pos Y", Y, PROPTYPE_INT, -1000000, 1000000},
		{"Color", Color, PROPTYPE_COLOR, 0, 0},
		{"Tex U", TextureU, PROPTYPE_INT, -1000000, 1000000},
		{"Tex V", TextureV, PROPTYPE_INT, -1000000, 1000000},
		{nullptr},
	};

	static int s_aIds[(int)EQuadPointProp::NUM_PROPS] = {0};
	int NewVal = 0;
	auto PropRes = pEditor->DoPropertiesWithState<EQuadPointProp>(&View, aProps, s_aIds, &NewVal);
	EQuadPointProp Prop = PropRes.m_Value;
	if(Prop != EQuadPointProp::PROP_NONE)
	{
		pEditor->m_Map.OnModify();
		if(PropRes.m_State == EEditState::START || PropRes.m_State == EEditState::ONE_GO)
		{
			pEditor->m_QuadTracker.BeginQuadPointPropTrack(pLayer, pEditor->m_vSelectedQuads, pEditor->m_SelectedQuadPoints);
			pEditor->m_QuadTracker.AddQuadPointPropTrack(Prop);
		}
	}

	for(CQuad *pQuad : vpQuads)
	{
		if(Prop == EQuadPointProp::PROP_POS_X)
		{
			for(int v = 0; v < 4; v++)
				if(pEditor->IsQuadCornerSelected(v))
					pQuad->m_aPoints[v].x = i2fx(fx2i(pQuad->m_aPoints[v].x) + NewVal - X);
		}
		else if(Prop == EQuadPointProp::PROP_POS_Y)
		{
			for(int v = 0; v < 4; v++)
				if(pEditor->IsQuadCornerSelected(v))
					pQuad->m_aPoints[v].y = i2fx(fx2i(pQuad->m_aPoints[v].y) + NewVal - Y);
		}
		else if(Prop == EQuadPointProp::PROP_COLOR)
		{
			for(int v = 0; v < 4; v++)
			{
				if(pEditor->IsQuadCornerSelected(v))
				{
					pQuad->m_aColors[v].r = (NewVal >> 24) & 0xff;
					pQuad->m_aColors[v].g = (NewVal >> 16) & 0xff;
					pQuad->m_aColors[v].b = (NewVal >> 8) & 0xff;
					pQuad->m_aColors[v].a = NewVal & 0xff;
				}
			}
		}
		else if(Prop == EQuadPointProp::PROP_TEX_U)
		{
			for(int v = 0; v < 4; v++)
				if(pEditor->IsQuadCornerSelected(v))
					pQuad->m_aTexcoords[v].x = f2fx(fx2f(pQuad->m_aTexcoords[v].x) + (NewVal - TextureU) / 1024.0f);
		}
		else if(Prop == EQuadPointProp::PROP_TEX_V)
		{
			for(int v = 0; v < 4; v++)
				if(pEditor->IsQuadCornerSelected(v))
					pQuad->m_aTexcoords[v].y = f2fx(fx2f(pQuad->m_aTexcoords[v].y) + (NewVal - TextureV) / 1024.0f);
		}
	}

	if(Prop != EQuadPointProp::PROP_NONE)
	{
		pEditor->m_Map.OnModify();
		if(PropRes.m_State == EEditState::END || PropRes.m_State == EEditState::ONE_GO)
		{
			pEditor->m_QuadTracker.EndQuadPointPropTrack(Prop);
		}
	}

	return CUI::POPUP_KEEP_OPEN;
}

CUI::EPopupMenuFunctionResult CEditor::PopupEnvPoint(void *pContext, CUIRect View, bool Active)
{
	CEditor *pEditor = static_cast<CEditor *>(pContext);
	if(pEditor->m_SelectedEnvelope < 0 || pEditor->m_SelectedEnvelope >= (int)pEditor->m_Map.m_vpEnvelopes.size())
		return CUI::POPUP_CLOSE_CURRENT;

	const float RowHeight = 12.0f;
	CUIRect Row, Label, EditBox;

	pEditor->m_ShowEnvelopePreview = SHOWENV_SELECTED;

	std::shared_ptr<CEnvelope> pEnvelope = pEditor->m_Map.m_vpEnvelopes[pEditor->m_SelectedEnvelope];

	if(pEnvelope->GetChannels() == 4)
	{
		View.HSplitTop(RowHeight, &Row, &View);
		View.HSplitTop(4.0f, nullptr, &View);
		Row.VSplitLeft(60.0f, &Label, &Row);
		Row.VSplitLeft(10.0f, nullptr, &EditBox);
		pEditor->UI()->DoLabel(&Label, "Color:", RowHeight - 2.0f, TEXTALIGN_ML);

		const auto SelectedPoint = pEditor->m_vSelectedEnvelopePoints.front();
		const int SelectedIndex = SelectedPoint.first;
		auto *pValues = pEnvelope->m_vPoints[SelectedIndex].m_aValues;
		const ColorRGBA Color = ColorRGBA(fx2f(pValues[0]), fx2f(pValues[1]), fx2f(pValues[2]), fx2f(pValues[3]));
		const auto &&SetColor = [&](ColorRGBA NewColor) {
			if(Color == NewColor && pEditor->m_ColorPickerPopupContext.m_State == EEditState::EDITING)
				return;

			static int s_Values[4];

			if(pEditor->m_ColorPickerPopupContext.m_State == EEditState::START || pEditor->m_ColorPickerPopupContext.m_State == EEditState::ONE_GO)
			{
				for(int Channel = 0; Channel < 4; ++Channel)
					s_Values[Channel] = pValues[Channel];
			}

			for(int Channel = 0; Channel < 4; ++Channel)
			{
				pValues[Channel] = f2fx(NewColor[Channel]);
			}

			if(pEditor->m_ColorPickerPopupContext.m_State == EEditState::END || pEditor->m_ColorPickerPopupContext.m_State == EEditState::ONE_GO)
			{
				std::vector<std::shared_ptr<IEditorAction>> vpActions(4);

				for(int Channel = 0; Channel < 4; ++Channel)
				{
					vpActions[Channel] = std::make_shared<CEditorActionEnvelopeEditPoint>(pEditor, pEditor->m_SelectedEnvelope, SelectedIndex, Channel, CEditorActionEnvelopeEditPoint::EEditType::VALUE, s_Values[Channel], f2fx(NewColor[Channel]));
				}

				char aDisplay[256];
				str_format(aDisplay, sizeof(aDisplay), "Edit color of point %d of envelope %d", SelectedIndex, pEditor->m_SelectedEnvelope);
				pEditor->m_EnvelopeEditorHistory.RecordAction(std::make_shared<CEditorActionBulk>(pEditor, vpActions, aDisplay));
			}

			pEditor->m_UpdateEnvPointInfo = true;
			pEditor->m_Map.OnModify();
		};
		static char s_ColorPickerButton;
		pEditor->DoColorPickerButton(&s_ColorPickerButton, &EditBox, Color, SetColor);
	}

	static CLineInputNumber s_CurValueInput;
	static CLineInputNumber s_CurTimeInput;

	static float s_CurrentTime = 0;
	static float s_CurrentValue = 0;

	if(pEditor->m_UpdateEnvPointInfo)
	{
		pEditor->m_UpdateEnvPointInfo = false;

		auto TimeAndValue = pEditor->EnvGetSelectedTimeAndValue();
		int CurrentTime = TimeAndValue.first;
		int CurrentValue = TimeAndValue.second;

		// update displayed text
		s_CurValueInput.SetFloat(fx2f(CurrentValue));
		s_CurTimeInput.SetFloat(CurrentTime / 1000.0f);

		s_CurrentTime = s_CurTimeInput.GetFloat();
		s_CurrentValue = s_CurValueInput.GetFloat();
	}

	View.HSplitTop(RowHeight, &Row, &View);
	Row.VSplitLeft(60.0f, &Label, &Row);
	Row.VSplitLeft(10.0f, nullptr, &EditBox);
	pEditor->UI()->DoLabel(&Label, "Value:", RowHeight - 2.0f, TEXTALIGN_ML);
	pEditor->DoEditBox(&s_CurValueInput, &EditBox, RowHeight - 2.0f, IGraphics::CORNER_ALL, "The value of the selected envelope point");

	View.HSplitTop(4.0f, nullptr, &View);
	View.HSplitTop(RowHeight, &Row, &View);
	Row.VSplitLeft(60.0f, &Label, &Row);
	Row.VSplitLeft(10.0f, nullptr, &EditBox);
	pEditor->UI()->DoLabel(&Label, "Time (in s):", RowHeight - 2.0f, TEXTALIGN_ML);
	pEditor->DoEditBox(&s_CurTimeInput, &EditBox, RowHeight - 2.0f, IGraphics::CORNER_ALL, "The time of the selected envelope point");

	if(pEditor->Input()->KeyIsPressed(KEY_RETURN) || pEditor->Input()->KeyIsPressed(KEY_KP_ENTER))
	{
		float CurrentTime = s_CurTimeInput.GetFloat();
		float CurrentValue = s_CurValueInput.GetFloat();
		if(!(absolute(CurrentTime - s_CurrentTime) < 0.0001f && absolute(CurrentValue - s_CurrentValue) < 0.0001f))
		{
			auto [OldTime, OldValue] = pEditor->EnvGetSelectedTimeAndValue();

			if(pEditor->IsTangentInSelected())
			{
				auto [SelectedIndex, SelectedChannel] = pEditor->m_SelectedTangentInPoint;

				pEditor->m_EnvelopeEditorHistory.Execute(std::make_shared<CEditorActionEditEnvelopePointValue>(pEditor, pEditor->m_SelectedEnvelope, SelectedIndex, SelectedChannel, CEditorActionEditEnvelopePointValue::EType::TANGENT_IN, static_cast<int>(OldTime * 1000.0f), f2fx(OldValue), static_cast<int>(CurrentTime * 1000.0f), f2fx(CurrentValue)));
				CurrentTime = (pEnvelope->m_vPoints[SelectedIndex].m_Time + pEnvelope->m_vPoints[SelectedIndex].m_Bezier.m_aInTangentDeltaX[SelectedChannel]) / 1000.0f;
			}
			else if(pEditor->IsTangentOutSelected())
			{
				auto [SelectedIndex, SelectedChannel] = pEditor->m_SelectedTangentOutPoint;

				pEditor->m_EnvelopeEditorHistory.Execute(std::make_shared<CEditorActionEditEnvelopePointValue>(pEditor, pEditor->m_SelectedEnvelope, SelectedIndex, SelectedChannel, CEditorActionEditEnvelopePointValue::EType::TANGENT_OUT, static_cast<int>(OldTime * 1000.0f), f2fx(OldValue), static_cast<int>(CurrentTime * 1000.0f), f2fx(CurrentValue)));
				CurrentTime = (pEnvelope->m_vPoints[SelectedIndex].m_Time + pEnvelope->m_vPoints[SelectedIndex].m_Bezier.m_aOutTangentDeltaX[SelectedChannel]) / 1000.0f;
			}
			else
			{
				auto [SelectedIndex, SelectedChannel] = pEditor->m_vSelectedEnvelopePoints.front();
				pEditor->m_EnvelopeEditorHistory.Execute(std::make_shared<CEditorActionEditEnvelopePointValue>(pEditor, pEditor->m_SelectedEnvelope, SelectedIndex, SelectedChannel, CEditorActionEditEnvelopePointValue::EType::POINT, static_cast<int>(OldTime * 1000.0f), f2fx(OldValue), static_cast<int>(CurrentTime * 1000.0f), f2fx(CurrentValue)));

				if(SelectedIndex != 0)
				{
					CurrentTime = pEnvelope->m_vPoints[SelectedIndex].m_Time / 1000.0f;
				}
				else
				{
					CurrentTime = 0.0f;
					pEnvelope->m_vPoints[SelectedIndex].m_Time = 0.0f;
				}
			}

			s_CurTimeInput.SetFloat(static_cast<int>(CurrentTime * 1000.0f) / 1000.0f);
			s_CurValueInput.SetFloat(fx2f(f2fx(CurrentValue)));

			s_CurrentTime = s_CurTimeInput.GetFloat();
			s_CurrentValue = s_CurValueInput.GetFloat();
		}
	}

	View.HSplitTop(6.0f, nullptr, &View);
	View.HSplitTop(RowHeight, &Row, &View);
	static int s_DeleteButtonID = 0;
	const char *pButtonText = pEditor->IsTangentSelected() ? "Reset" : "Delete";
	const char *pTooltip = pEditor->IsTangentSelected() ? "Reset tangent point to default value." : "Delete current envelope point in all channels.";
	if(pEditor->DoButton_Editor(&s_DeleteButtonID, pButtonText, 0, &Row, 0, pTooltip))
	{
		if(pEditor->IsTangentInSelected())
		{
			auto [SelectedIndex, SelectedChannel] = pEditor->m_SelectedTangentInPoint;
			pEditor->m_EnvelopeEditorHistory.Execute(std::make_shared<CEditorActionResetEnvelopePointTangent>(pEditor, pEditor->m_SelectedEnvelope, SelectedIndex, SelectedChannel, true));
		}
		else if(pEditor->IsTangentOutSelected())
		{
			auto [SelectedIndex, SelectedChannel] = pEditor->m_SelectedTangentOutPoint;
			pEditor->m_EnvelopeEditorHistory.Execute(std::make_shared<CEditorActionResetEnvelopePointTangent>(pEditor, pEditor->m_SelectedEnvelope, SelectedIndex, SelectedChannel, false));
		}
		else
		{
			auto [SelectedIndex, SelectedChannel] = pEditor->m_vSelectedEnvelopePoints.front();
			pEditor->m_EnvelopeEditorHistory.Execute(std::make_shared<CEditorActionDeleteEnvelopePoint>(pEditor, pEditor->m_SelectedEnvelope, SelectedIndex));
		}

		return CUI::POPUP_CLOSE_CURRENT;
	}

	return CUI::POPUP_KEEP_OPEN;
}

CUI::EPopupMenuFunctionResult CEditor::PopupEnvPointMulti(void *pContext, CUIRect View, bool Active)
{
	CEditor *pEditor = static_cast<CEditor *>(pContext);
	const float RowHeight = 12.0f;

	static int s_CurveButtonID = 0;
	CUIRect CurveButton;
	View.HSplitTop(RowHeight, &CurveButton, &View);
	if(pEditor->DoButton_Editor(&s_CurveButtonID, "Project onto", 0, &CurveButton, 0, "Project all selected envelopes onto the curve between the first and last selected envelope."))
	{
		static SPopupMenuId s_PopupCurveTypeId;
		pEditor->UI()->DoPopupMenu(&s_PopupCurveTypeId, pEditor->UI()->MouseX(), pEditor->UI()->MouseY(), 80, 80, pEditor, PopupEnvPointCurveType);
	}

	return CUI::POPUP_KEEP_OPEN;
}

CUI::EPopupMenuFunctionResult CEditor::PopupEnvPointCurveType(void *pContext, CUIRect View, bool Active)
{
	CEditor *pEditor = static_cast<CEditor *>(pContext);
	const float RowHeight = 14.0f;

	int CurveType = -1;

	static int s_ButtonLinearID;
	CUIRect ButtonLinear;
	View.HSplitTop(RowHeight, &ButtonLinear, &View);
	if(pEditor->DoButton_MenuItem(&s_ButtonLinearID, "Linear", 0, &ButtonLinear))
		CurveType = CURVETYPE_LINEAR;

	static int s_ButtonSlowID;
	CUIRect ButtonSlow;
	View.HSplitTop(RowHeight, &ButtonSlow, &View);
	if(pEditor->DoButton_MenuItem(&s_ButtonSlowID, "Slow", 0, &ButtonSlow))
		CurveType = CURVETYPE_SLOW;

	static int s_ButtonFastID;
	CUIRect ButtonFast;
	View.HSplitTop(RowHeight, &ButtonFast, &View);
	if(pEditor->DoButton_MenuItem(&s_ButtonFastID, "Fast", 0, &ButtonFast))
		CurveType = CURVETYPE_FAST;

	static int s_ButtonStepID;
	CUIRect ButtonStep;
	View.HSplitTop(RowHeight, &ButtonStep, &View);
	if(pEditor->DoButton_MenuItem(&s_ButtonStepID, "Step", 0, &ButtonStep))
		CurveType = CURVETYPE_STEP;

	static int s_ButtonSmoothID;
	CUIRect ButtonSmooth;
	View.HSplitTop(RowHeight, &ButtonSmooth, &View);
	if(pEditor->DoButton_MenuItem(&s_ButtonSmoothID, "Smooth", 0, &ButtonSmooth))
		CurveType = CURVETYPE_SMOOTH;

	std::vector<std::shared_ptr<IEditorAction>> vpActions;

	if(CurveType >= 0)
	{
		std::shared_ptr<CEnvelope> pEnvelope = pEditor->m_Map.m_vpEnvelopes.at(pEditor->m_SelectedEnvelope);

		for(int c = 0; c < pEnvelope->GetChannels(); c++)
		{
			int FirstSelectedIndex = pEnvelope->m_vPoints.size();
			int LastSelectedIndex = -1;
			for(auto [SelectedIndex, SelectedChannel] : pEditor->m_vSelectedEnvelopePoints)
			{
				if(SelectedChannel == c)
				{
					FirstSelectedIndex = minimum(FirstSelectedIndex, SelectedIndex);
					LastSelectedIndex = maximum(LastSelectedIndex, SelectedIndex);
				}
			}

			if(FirstSelectedIndex < (int)pEnvelope->m_vPoints.size() && LastSelectedIndex >= 0 && FirstSelectedIndex != LastSelectedIndex)
			{
				CEnvPoint FirstPoint = pEnvelope->m_vPoints[FirstSelectedIndex];
				CEnvPoint LastPoint = pEnvelope->m_vPoints[LastSelectedIndex];

				CEnvelope HelperEnvelope(1);
				HelperEnvelope.AddPoint(FirstPoint.m_Time, FirstPoint.m_aValues[c]);
				HelperEnvelope.AddPoint(LastPoint.m_Time, LastPoint.m_aValues[c]);
				HelperEnvelope.m_vPoints[0].m_Curvetype = CurveType;

				for(auto [SelectedIndex, SelectedChannel] : pEditor->m_vSelectedEnvelopePoints)
				{
					if(SelectedChannel == c)
					{
						if(SelectedIndex != FirstSelectedIndex && SelectedIndex != LastSelectedIndex)
						{
							CEnvPoint &CurrentPoint = pEnvelope->m_vPoints[SelectedIndex];
							ColorRGBA Channels;
							HelperEnvelope.Eval(CurrentPoint.m_Time / 1000.0f, Channels);
							int PrevValue = CurrentPoint.m_aValues[c];
							CurrentPoint.m_aValues[c] = f2fx(Channels.r);
							vpActions.push_back(std::make_shared<CEditorActionEnvelopeEditPoint>(pEditor, pEditor->m_SelectedEnvelope, SelectedIndex, SelectedChannel, CEditorActionEnvelopeEditPoint::EEditType::VALUE, PrevValue, CurrentPoint.m_aValues[c]));
						}
					}
				}
			}
		}

		if(!vpActions.empty())
		{
			pEditor->m_EnvelopeEditorHistory.RecordAction(std::make_shared<CEditorActionBulk>(pEditor, vpActions, "Project points"));
		}

		pEditor->m_Map.OnModify();
		return CUI::POPUP_CLOSE_CURRENT;
	}

	return CUI::POPUP_KEEP_OPEN;
}

static const auto &&gs_ModifyIndexDeleted = [](int DeletedIndex) {
	return [DeletedIndex](int *pIndex) {
		if(*pIndex == DeletedIndex)
			*pIndex = -1;
		else if(*pIndex > DeletedIndex)
			*pIndex = *pIndex - 1;
	};
};

CUI::EPopupMenuFunctionResult CEditor::PopupImage(void *pContext, CUIRect View, bool Active)
{
	CEditor *pEditor = static_cast<CEditor *>(pContext);

	static int s_ExternalButton = 0;
	static int s_ReaddButton = 0;
	static int s_ReplaceButton = 0;
	static int s_RemoveButton = 0;
	static int s_ExportButton = 0;

	const float RowHeight = 12.0f;

	CUIRect Slot;
	View.HSplitTop(RowHeight, &Slot, &View);
	std::shared_ptr<CEditorImage> pImg = pEditor->m_Map.m_vpImages[pEditor->m_SelectedImage];

	if(!pImg->m_External)
	{
		CUIRect Label, EditBox;

		static CLineInput s_RenameInput;

		Slot.VMargin(5.0f, &Slot);
		Slot.VSplitLeft(35.0f, &Label, &Slot);
		Slot.VSplitLeft(RowHeight - 2.0f, nullptr, &EditBox);
		pEditor->UI()->DoLabel(&Label, "Name:", RowHeight - 2.0f, TEXTALIGN_ML);

		s_RenameInput.SetBuffer(pImg->m_aName, sizeof(pImg->m_aName));
		if(pEditor->DoEditBox(&s_RenameInput, &EditBox, RowHeight - 2.0f))
			pEditor->m_Map.OnModify();

		View.HSplitTop(5.0f, nullptr, &View);
		View.HSplitTop(RowHeight, &Slot, &View);
	}

	if(pImg->m_External)
	{
		if(pEditor->DoButton_MenuItem(&s_ExternalButton, "Embed", 0, &Slot, 0, "Embeds the image into the map file."))
		{
			pImg->m_External = 0;
			return CUI::POPUP_CLOSE_CURRENT;
		}
		View.HSplitTop(5.0f, nullptr, &View);
		View.HSplitTop(RowHeight, &Slot, &View);
	}
	else if(CEditor::IsVanillaImage(pImg->m_aName))
	{
		if(pEditor->DoButton_MenuItem(&s_ExternalButton, "Make external", 0, &Slot, 0, "Removes the image from the map file."))
		{
			pImg->m_External = 1;
			return CUI::POPUP_CLOSE_CURRENT;
		}
		View.HSplitTop(5.0f, nullptr, &View);
		View.HSplitTop(RowHeight, &Slot, &View);
	}

	static CUI::SSelectionPopupContext s_SelectionPopupContext;
	static CScrollRegion s_SelectionPopupScrollRegion;
	s_SelectionPopupContext.m_pScrollRegion = &s_SelectionPopupScrollRegion;
	if(pEditor->DoButton_MenuItem(&s_ReaddButton, "Readd", 0, &Slot, 0, "Reloads the image from the mapres folder"))
	{
		char aFilename[IO_MAX_PATH_LENGTH];
		str_format(aFilename, sizeof(aFilename), "%s.png", pImg->m_aName);
		s_SelectionPopupContext.Reset();
		std::set<std::string> EntriesSet;
		pEditor->Storage()->FindFiles(aFilename, "mapres", IStorage::TYPE_ALL, &EntriesSet);
		for(const auto &Entry : EntriesSet)
			s_SelectionPopupContext.m_vEntries.push_back(Entry);
		if(s_SelectionPopupContext.m_vEntries.empty())
		{
			pEditor->ShowFileDialogError("Error: could not find image '%s' in the mapres folder.", aFilename);
		}
		else if(s_SelectionPopupContext.m_vEntries.size() == 1)
		{
			s_SelectionPopupContext.m_pSelection = &s_SelectionPopupContext.m_vEntries.front();
		}
		else
		{
			str_copy(s_SelectionPopupContext.m_aMessage, "Select the wanted image:");
			pEditor->UI()->ShowPopupSelection(pEditor->UI()->MouseX(), pEditor->UI()->MouseY(), &s_SelectionPopupContext);
		}
	}
	if(s_SelectionPopupContext.m_pSelection != nullptr)
	{
		const bool WasExternal = pImg->m_External;
		const bool Result = pEditor->ReplaceImage(s_SelectionPopupContext.m_pSelection->c_str(), IStorage::TYPE_ALL, false);
		pImg->m_External = WasExternal;
		s_SelectionPopupContext.Reset();
		return Result ? CUI::POPUP_CLOSE_CURRENT : CUI::POPUP_KEEP_OPEN;
	}

	View.HSplitTop(5.0f, nullptr, &View);
	View.HSplitTop(RowHeight, &Slot, &View);
	if(pEditor->DoButton_MenuItem(&s_ReplaceButton, "Replace", 0, &Slot, 0, "Replaces the image with a new one"))
	{
		pEditor->InvokeFileDialog(IStorage::TYPE_ALL, FILETYPE_IMG, "Replace Image", "Replace", "mapres", false, ReplaceImageCallback, pEditor);
		return CUI::POPUP_CLOSE_CURRENT;
	}

	View.HSplitTop(5.0f, nullptr, &View);
	View.HSplitTop(RowHeight, &Slot, &View);
	if(pEditor->DoButton_MenuItem(&s_RemoveButton, "Remove", 0, &Slot, 0, "Removes the image from the map"))
	{
		pEditor->m_Map.m_vpImages.erase(pEditor->m_Map.m_vpImages.begin() + pEditor->m_SelectedImage);
		pEditor->m_Map.ModifyImageIndex(gs_ModifyIndexDeleted(pEditor->m_SelectedImage));
		return CUI::POPUP_CLOSE_CURRENT;
	}

	if(!pImg->m_External)
	{
		View.HSplitTop(5.0f, nullptr, &View);
		View.HSplitTop(RowHeight, &Slot, &View);
		if(pEditor->DoButton_MenuItem(&s_ExportButton, "Export", 0, &Slot, 0, "Export the image"))
		{
			pEditor->InvokeFileDialog(IStorage::TYPE_SAVE, FILETYPE_IMG, "Save image", "Save", "mapres", false, CallbackSaveImage, pEditor);
			pEditor->m_FileDialogFileNameInput.Set(pImg->m_aName);
			return CUI::POPUP_CLOSE_CURRENT;
		}
	}

	return CUI::POPUP_KEEP_OPEN;
}

CUI::EPopupMenuFunctionResult CEditor::PopupSound(void *pContext, CUIRect View, bool Active)
{
	CEditor *pEditor = static_cast<CEditor *>(pContext);

	static int s_ReaddButton = 0;
	static int s_ReplaceButton = 0;
	static int s_RemoveButton = 0;
	static int s_ExportButton = 0;

	const float RowHeight = 12.0f;

	CUIRect Slot;
	View.HSplitTop(RowHeight, &Slot, &View);
	std::shared_ptr<CEditorSound> pSound = pEditor->m_Map.m_vpSounds[pEditor->m_SelectedSound];

	static CUI::SSelectionPopupContext s_SelectionPopupContext;
	static CScrollRegion s_SelectionPopupScrollRegion;
	s_SelectionPopupContext.m_pScrollRegion = &s_SelectionPopupScrollRegion;

	CUIRect Label, EditBox;

	static CLineInput s_RenameInput;

	Slot.VMargin(5.0f, &Slot);
	Slot.VSplitLeft(35.0f, &Label, &Slot);
	Slot.VSplitLeft(RowHeight - 2.0f, nullptr, &EditBox);
	pEditor->UI()->DoLabel(&Label, "Name:", RowHeight - 2.0f, TEXTALIGN_ML);

	s_RenameInput.SetBuffer(pSound->m_aName, sizeof(pSound->m_aName));
	if(pEditor->DoEditBox(&s_RenameInput, &EditBox, RowHeight - 2.0f))
		pEditor->m_Map.OnModify();

	View.HSplitTop(5.0f, nullptr, &View);
	View.HSplitTop(RowHeight, &Slot, &View);

	if(pEditor->DoButton_MenuItem(&s_ReaddButton, "Readd", 0, &Slot, 0, "Reloads the sound from the mapres folder"))
	{
		char aFilename[IO_MAX_PATH_LENGTH];
		str_format(aFilename, sizeof(aFilename), "%s.opus", pSound->m_aName);
		s_SelectionPopupContext.Reset();
		std::set<std::string> EntriesSet;
		pEditor->Storage()->FindFiles(aFilename, "mapres", IStorage::TYPE_ALL, &EntriesSet);
		for(const auto &Entry : EntriesSet)
			s_SelectionPopupContext.m_vEntries.push_back(Entry);
		if(s_SelectionPopupContext.m_vEntries.empty())
		{
			pEditor->ShowFileDialogError("Error: could not find sound '%s' in the mapres folder.", aFilename);
		}
		else if(s_SelectionPopupContext.m_vEntries.size() == 1)
		{
			s_SelectionPopupContext.m_pSelection = &s_SelectionPopupContext.m_vEntries.front();
		}
		else
		{
			str_copy(s_SelectionPopupContext.m_aMessage, "Select the wanted sound:");
			pEditor->UI()->ShowPopupSelection(pEditor->UI()->MouseX(), pEditor->UI()->MouseY(), &s_SelectionPopupContext);
		}
	}
	if(s_SelectionPopupContext.m_pSelection != nullptr)
	{
		const bool Result = pEditor->ReplaceSound(s_SelectionPopupContext.m_pSelection->c_str(), IStorage::TYPE_ALL, false);
		s_SelectionPopupContext.Reset();
		return Result ? CUI::POPUP_CLOSE_CURRENT : CUI::POPUP_KEEP_OPEN;
	}

	View.HSplitTop(5.0f, nullptr, &View);
	View.HSplitTop(RowHeight, &Slot, &View);
	if(pEditor->DoButton_MenuItem(&s_ReplaceButton, "Replace", 0, &Slot, 0, "Replaces the sound with a new one"))
	{
		pEditor->InvokeFileDialog(IStorage::TYPE_ALL, FILETYPE_SOUND, "Replace sound", "Replace", "mapres", false, ReplaceSoundCallback, pEditor);
		return CUI::POPUP_CLOSE_CURRENT;
	}

	View.HSplitTop(5.0f, nullptr, &View);
	View.HSplitTop(RowHeight, &Slot, &View);
	if(pEditor->DoButton_MenuItem(&s_RemoveButton, "Remove", 0, &Slot, 0, "Removes the sound from the map"))
	{
		pEditor->m_Map.m_vpSounds.erase(pEditor->m_Map.m_vpSounds.begin() + pEditor->m_SelectedSound);
		pEditor->m_Map.ModifySoundIndex(gs_ModifyIndexDeleted(pEditor->m_SelectedSound));
		return CUI::POPUP_CLOSE_CURRENT;
	}

	View.HSplitTop(5.0f, nullptr, &View);
	View.HSplitTop(RowHeight, &Slot, &View);
	if(pEditor->DoButton_MenuItem(&s_ExportButton, "Export", 0, &Slot, 0, "Export sound"))
	{
		pEditor->InvokeFileDialog(IStorage::TYPE_SAVE, FILETYPE_SOUND, "Save sound", "Save", "mapres", false, CallbackSaveSound, pEditor);
		pEditor->m_FileDialogFileNameInput.Set(pSound->m_aName);
		return CUI::POPUP_CLOSE_CURRENT;
	}

	return CUI::POPUP_KEEP_OPEN;
}

CUI::EPopupMenuFunctionResult CEditor::PopupNewFolder(void *pContext, CUIRect View, bool Active)
{
	CEditor *pEditor = static_cast<CEditor *>(pContext);

	CUIRect Label, ButtonBar, Button;

	View.Margin(10.0f, &View);
	View.HSplitBottom(20.0f, &View, &ButtonBar);

	// title
	View.HSplitTop(20.0f, &Label, &View);
	pEditor->UI()->DoLabel(&Label, "Create new folder", 20.0f, TEXTALIGN_MC);
	View.HSplitTop(10.0f, nullptr, &View);

	// folder name
	View.HSplitTop(20.0f, &Label, &View);
	pEditor->UI()->DoLabel(&Label, "Name:", 10.0f, TEXTALIGN_ML);
	Label.VSplitLeft(50.0f, nullptr, &Button);
	Button.HMargin(2.0f, &Button);
	pEditor->DoEditBox(&pEditor->m_FileDialogNewFolderNameInput, &Button, 12.0f);

	// button bar
	ButtonBar.VSplitLeft(110.0f, &Button, &ButtonBar);
	static int s_CancelButton = 0;
	if(pEditor->DoButton_Editor(&s_CancelButton, "Cancel", 0, &Button, 0, nullptr))
		return CUI::POPUP_CLOSE_CURRENT;

	ButtonBar.VSplitRight(110.0f, &ButtonBar, &Button);
	static int s_CreateButton = 0;
	if(pEditor->DoButton_Editor(&s_CreateButton, "Create", 0, &Button, 0, nullptr) || (Active && pEditor->UI()->ConsumeHotkey(CUI::HOTKEY_ENTER)))
	{
		// create the folder
		if(!pEditor->m_FileDialogNewFolderNameInput.IsEmpty())
		{
			char aBuf[IO_MAX_PATH_LENGTH];
			str_format(aBuf, sizeof(aBuf), "%s/%s", pEditor->m_pFileDialogPath, pEditor->m_FileDialogNewFolderNameInput.GetString());
			if(pEditor->Storage()->CreateFolder(aBuf, IStorage::TYPE_SAVE))
			{
				pEditor->FilelistPopulate(IStorage::TYPE_SAVE);
				return CUI::POPUP_CLOSE_CURRENT;
			}
			else
			{
				pEditor->ShowFileDialogError("Failed to create the folder '%s'.", aBuf);
			}
		}
	}

	return CUI::POPUP_KEEP_OPEN;
}

CUI::EPopupMenuFunctionResult CEditor::PopupMapInfo(void *pContext, CUIRect View, bool Active)
{
	CEditor *pEditor = static_cast<CEditor *>(pContext);

	CUIRect Label, ButtonBar, Button;

	View.Margin(10.0f, &View);
	View.HSplitBottom(20.0f, &View, &ButtonBar);

	// title
	View.HSplitTop(20.0f, &Label, &View);
	pEditor->UI()->DoLabel(&Label, "Map details", 20.0f, TEXTALIGN_MC);
	View.HSplitTop(10.0f, nullptr, &View);

	// author box
	View.HSplitTop(20.0f, &Label, &View);
	pEditor->UI()->DoLabel(&Label, "Author:", 10.0f, TEXTALIGN_ML);
	Label.VSplitLeft(60.0f, nullptr, &Button);
	Button.HMargin(3.0f, &Button);
	static CLineInput s_AuthorInput;
	s_AuthorInput.SetBuffer(pEditor->m_Map.m_MapInfoTmp.m_aAuthor, sizeof(pEditor->m_Map.m_MapInfoTmp.m_aAuthor));
	pEditor->DoEditBox(&s_AuthorInput, &Button, 10.0f);

	// version box
	View.HSplitTop(20.0f, &Label, &View);
	pEditor->UI()->DoLabel(&Label, "Version:", 10.0f, TEXTALIGN_ML);
	Label.VSplitLeft(60.0f, nullptr, &Button);
	Button.HMargin(3.0f, &Button);
	static CLineInput s_VersionInput;
	s_VersionInput.SetBuffer(pEditor->m_Map.m_MapInfoTmp.m_aVersion, sizeof(pEditor->m_Map.m_MapInfoTmp.m_aVersion));
	pEditor->DoEditBox(&s_VersionInput, &Button, 10.0f);

	// credits box
	View.HSplitTop(20.0f, &Label, &View);
	pEditor->UI()->DoLabel(&Label, "Credits:", 10.0f, TEXTALIGN_ML);
	Label.VSplitLeft(60.0f, nullptr, &Button);
	Button.HMargin(3.0f, &Button);
	static CLineInput s_CreditsInput;
	s_CreditsInput.SetBuffer(pEditor->m_Map.m_MapInfoTmp.m_aCredits, sizeof(pEditor->m_Map.m_MapInfoTmp.m_aCredits));
	pEditor->DoEditBox(&s_CreditsInput, &Button, 10.0f);

	// license box
	View.HSplitTop(20.0f, &Label, &View);
	pEditor->UI()->DoLabel(&Label, "License:", 10.0f, TEXTALIGN_ML);
	Label.VSplitLeft(60.0f, nullptr, &Button);
	Button.HMargin(3.0f, &Button);
	static CLineInput s_LicenseInput;
	s_LicenseInput.SetBuffer(pEditor->m_Map.m_MapInfoTmp.m_aLicense, sizeof(pEditor->m_Map.m_MapInfoTmp.m_aLicense));
	pEditor->DoEditBox(&s_LicenseInput, &Button, 10.0f);

	// button bar
	ButtonBar.VSplitLeft(110.0f, &Label, &ButtonBar);
	static int s_CancelButton = 0;
	if(pEditor->DoButton_Editor(&s_CancelButton, "Cancel", 0, &Label, 0, nullptr))
		return CUI::POPUP_CLOSE_CURRENT;

	ButtonBar.VSplitRight(110.0f, &ButtonBar, &Label);
	static int s_ConfirmButton = 0;
	if(pEditor->DoButton_Editor(&s_ConfirmButton, "Confirm", 0, &Label, 0, nullptr) || (Active && pEditor->UI()->ConsumeHotkey(CUI::HOTKEY_ENTER)))
	{
		bool AuthorDifferent = str_comp(pEditor->m_Map.m_MapInfoTmp.m_aAuthor, pEditor->m_Map.m_MapInfo.m_aAuthor) != 0;
		bool VersionDifferent = str_comp(pEditor->m_Map.m_MapInfoTmp.m_aVersion, pEditor->m_Map.m_MapInfo.m_aVersion) != 0;
		bool CreditsDifferent = str_comp(pEditor->m_Map.m_MapInfoTmp.m_aCredits, pEditor->m_Map.m_MapInfo.m_aCredits) != 0;
		bool LicenseDifferent = str_comp(pEditor->m_Map.m_MapInfoTmp.m_aLicense, pEditor->m_Map.m_MapInfo.m_aLicense) != 0;

		if(AuthorDifferent || VersionDifferent || CreditsDifferent || LicenseDifferent)
			pEditor->m_Map.OnModify();

		pEditor->m_Map.m_MapInfo.Copy(pEditor->m_Map.m_MapInfoTmp);
		return CUI::POPUP_CLOSE_CURRENT;
	}

	return CUI::POPUP_KEEP_OPEN;
}

CUI::EPopupMenuFunctionResult CEditor::PopupEvent(void *pContext, CUIRect View, bool Active)
{
	CEditor *pEditor = static_cast<CEditor *>(pContext);

	const char *pTitle;
	const char *pMessage;
	char aMessageBuf[128];
	if(pEditor->m_PopupEventType == POPEVENT_EXIT)
	{
		pTitle = "Exit the editor";
		pMessage = "The map contains unsaved data, you might want to save it before you exit the editor.\n\nContinue anyway?";
	}
	else if(pEditor->m_PopupEventType == POPEVENT_LOAD || pEditor->m_PopupEventType == POPEVENT_LOADCURRENT || pEditor->m_PopupEventType == POPEVENT_LOADDROP)
	{
		pTitle = "Load map";
		pMessage = "The map contains unsaved data, you might want to save it before you load a new map.\n\nContinue anyway?";
	}
	else if(pEditor->m_PopupEventType == POPEVENT_NEW)
	{
		pTitle = "New map";
		pMessage = "The map contains unsaved data, you might want to save it before you create a new map.\n\nContinue anyway?";
	}
	else if(pEditor->m_PopupEventType == POPEVENT_SAVE || pEditor->m_PopupEventType == POPEVENT_SAVE_COPY)
	{
		pTitle = "Save map";
		pMessage = "The file already exists.\n\nDo you want to overwrite the map?";
	}
	else if(pEditor->m_PopupEventType == POPEVENT_SAVE_IMG)
	{
		pTitle = "Save image";
		pMessage = "The file already exists.\n\nDo you want to overwrite the image?";
	}
	else if(pEditor->m_PopupEventType == POPEVENT_SAVE_SOUND)
	{
		pTitle = "Save sound";
		pMessage = "The file already exists.\n\nDo you want to overwrite the sound?";
	}
	else if(pEditor->m_PopupEventType == POPEVENT_LARGELAYER)
	{
		pTitle = "Large layer";
		pMessage = "You are trying to set the height or width of a layer to more than 1000 tiles. This is actually possible, but only rarely necessary. It may cause the editor to work slower and will result in a larger file size as well as higher memory usage for client and server.";
	}
	else if(pEditor->m_PopupEventType == POPEVENT_PREVENTUNUSEDTILES)
	{
		pTitle = "Unused tiles disabled";
		pMessage = "Unused tiles can't be placed by default because they could get a use later and then destroy your map.\n\nActivate the 'Allow Unused' setting to be able to place every tile.";
	}
	else if(pEditor->m_PopupEventType == POPEVENT_IMAGEDIV16)
	{
		pTitle = "Image width/height";
		pMessage = "The width or height of this image is not divisible by 16. This is required for images used in tile layers.";
	}
	else if(pEditor->m_PopupEventType == POPEVENT_IMAGE_MAX)
	{
		pTitle = "Max images";
		str_format(aMessageBuf, sizeof(aMessageBuf), "The client only allows a maximum of %" PRIzu " images.", MAX_MAPIMAGES);
		pMessage = aMessageBuf;
	}
	else if(pEditor->m_PopupEventType == POPEVENT_SOUND_MAX)
	{
		pTitle = "Max sounds";
		str_format(aMessageBuf, sizeof(aMessageBuf), "The client only allows a maximum of %" PRIzu " sounds.", MAX_MAPSOUNDS);
		pMessage = aMessageBuf;
	}
	else if(pEditor->m_PopupEventType == POPEVENT_PLACE_BORDER_TILES)
	{
		pTitle = "Place border tiles";
		pMessage = "This is going to overwrite any existing tiles around the edges of the layer.\n\nContinue?";
	}
	else if(pEditor->m_PopupEventType == POPEVENT_PIXELART_BIG_IMAGE)
	{
		pTitle = "Big image";
		pMessage = "The selected image is big. Converting it to tileart may take some time.\n\nContinue anyway?";
	}
	else if(pEditor->m_PopupEventType == POPEVENT_PIXELART_MANY_COLORS)
	{
		pTitle = "Many colors";
		pMessage = "The selected image contains many colors, which will lead to a big mapfile. You may want to consider reducing the number of colors.\n\nContinue anyway?";
	}
	else if(pEditor->m_PopupEventType == POPEVENT_PIXELART_TOO_MANY_COLORS)
	{
		pTitle = "Too many colors";
		pMessage = "The client only supports 64 images but more would be needed to add the selected image as tileart.";
	}
	else
	{
		dbg_assert(false, "m_PopupEventType invalid");
		return CUI::POPUP_CLOSE_CURRENT;
	}

	CUIRect Label, ButtonBar, Button;

	View.Margin(10.0f, &View);
	View.HSplitBottom(20.0f, &View, &ButtonBar);

	// title
	View.HSplitTop(20.0f, &Label, &View);
	pEditor->UI()->DoLabel(&Label, pTitle, 20.0f, TEXTALIGN_MC);

	// message
	SLabelProperties Props;
	Props.m_MaxWidth = View.w;
	pEditor->UI()->DoLabel(&View, pMessage, 10.0f, TEXTALIGN_ML, Props);

	// button bar
	ButtonBar.VSplitLeft(110.0f, &Button, &ButtonBar);
	if(pEditor->m_PopupEventType != POPEVENT_LARGELAYER &&
		pEditor->m_PopupEventType != POPEVENT_PREVENTUNUSEDTILES &&
		pEditor->m_PopupEventType != POPEVENT_IMAGEDIV16 &&
		pEditor->m_PopupEventType != POPEVENT_IMAGE_MAX &&
		pEditor->m_PopupEventType != POPEVENT_SOUND_MAX &&
		pEditor->m_PopupEventType != POPEVENT_PIXELART_TOO_MANY_COLORS)
	{
		static int s_CancelButton = 0;
		if(pEditor->DoButton_Editor(&s_CancelButton, "Cancel", 0, &Button, 0, nullptr))
		{
			if(pEditor->m_PopupEventType == POPEVENT_LOADDROP)
				pEditor->m_aFileNamePending[0] = 0;
			pEditor->m_PopupEventWasActivated = false;

			if(pEditor->m_PopupEventType == POPEVENT_PIXELART_BIG_IMAGE || pEditor->m_PopupEventType == POPEVENT_PIXELART_MANY_COLORS)
			{
				free(pEditor->m_TileartImageInfo.m_pData);
				pEditor->m_TileartImageInfo.m_pData = nullptr;
			}

			return CUI::POPUP_CLOSE_CURRENT;
		}
	}

	ButtonBar.VSplitRight(110.0f, &ButtonBar, &Button);
	static int s_ConfirmButton = 0;
	if(pEditor->DoButton_Editor(&s_ConfirmButton, "Confirm", 0, &Button, 0, nullptr) || (Active && pEditor->UI()->ConsumeHotkey(CUI::HOTKEY_ENTER)))
	{
		if(pEditor->m_PopupEventType == POPEVENT_EXIT)
		{
			pEditor->OnClose();
			g_Config.m_ClEditor = 0;
		}
		else if(pEditor->m_PopupEventType == POPEVENT_LOAD)
		{
			pEditor->InvokeFileDialog(IStorage::TYPE_ALL, FILETYPE_MAP, "Load map", "Load", "maps", false, CEditor::CallbackOpenMap, pEditor);
		}
		else if(pEditor->m_PopupEventType == POPEVENT_LOADCURRENT)
		{
			pEditor->LoadCurrentMap();
		}
		else if(pEditor->m_PopupEventType == POPEVENT_LOADDROP)
		{
			int Result = pEditor->Load(pEditor->m_aFileNamePending, IStorage::TYPE_ALL_OR_ABSOLUTE);
			if(!Result)
				dbg_msg("editor", "editing passed map file '%s' failed", pEditor->m_aFileNamePending);
			pEditor->m_aFileNamePending[0] = 0;
		}
		else if(pEditor->m_PopupEventType == POPEVENT_NEW)
		{
			pEditor->Reset();
			pEditor->m_aFileName[0] = 0;
		}
		else if(pEditor->m_PopupEventType == POPEVENT_SAVE)
		{
			CallbackSaveMap(pEditor->m_aFileSaveName, IStorage::TYPE_SAVE, pEditor);
			return CUI::POPUP_CLOSE_CURRENT;
		}
		else if(pEditor->m_PopupEventType == POPEVENT_SAVE_COPY)
		{
			CallbackSaveCopyMap(pEditor->m_aFileSaveName, IStorage::TYPE_SAVE, pEditor);
			return CUI::POPUP_CLOSE_CURRENT;
		}
		else if(pEditor->m_PopupEventType == POPEVENT_SAVE_IMG)
		{
			CallbackSaveImage(pEditor->m_aFileSaveName, IStorage::TYPE_SAVE, pEditor);
			return CUI::POPUP_CLOSE_CURRENT;
		}
		else if(pEditor->m_PopupEventType == POPEVENT_SAVE_SOUND)
		{
			CallbackSaveSound(pEditor->m_aFileSaveName, IStorage::TYPE_SAVE, pEditor);
			return CUI::POPUP_CLOSE_CURRENT;
		}
		else if(pEditor->m_PopupEventType == POPEVENT_PLACE_BORDER_TILES)
		{
			pEditor->PlaceBorderTiles();
		}
		else if(pEditor->m_PopupEventType == POPEVENT_PIXELART_BIG_IMAGE)
		{
			pEditor->TileartCheckColors();
		}
		else if(pEditor->m_PopupEventType == POPEVENT_PIXELART_MANY_COLORS)
		{
			pEditor->AddTileart();
		}
		pEditor->m_PopupEventWasActivated = false;
		return CUI::POPUP_CLOSE_CURRENT;
	}

	return CUI::POPUP_KEEP_OPEN;
}

static int g_SelectImageSelected = -100;
static int g_SelectImageCurrent = -100;

CUI::EPopupMenuFunctionResult CEditor::PopupSelectImage(void *pContext, CUIRect View, bool Active)
{
	CEditor *pEditor = static_cast<CEditor *>(pContext);

	CUIRect ButtonBar, ImageView;
	View.VSplitLeft(150.0f, &ButtonBar, &View);
	View.Margin(10.0f, &ImageView);

	int ShowImage = g_SelectImageCurrent;

	const float ButtonHeight = 12.0f;
	const float ButtonMargin = 2.0f;

	static CScrollRegion s_ScrollRegion;
	vec2 ScrollOffset(0.0f, 0.0f);
	CScrollRegionParams ScrollParams;
	ScrollParams.m_ScrollbarWidth = 10.0f;
	ScrollParams.m_ScrollbarMargin = 3.0f;
	ScrollParams.m_ScrollUnit = (ButtonHeight + ButtonMargin) * 5;
	s_ScrollRegion.Begin(&ButtonBar, &ScrollOffset, &ScrollParams);
	ButtonBar.y += ScrollOffset.y;

	for(int i = -1; i < (int)pEditor->m_Map.m_vpImages.size(); i++)
	{
		CUIRect Button;
		ButtonBar.HSplitTop(ButtonMargin, nullptr, &ButtonBar);
		ButtonBar.HSplitTop(ButtonHeight, &Button, &ButtonBar);
		if(s_ScrollRegion.AddRect(Button))
		{
			if(pEditor->UI()->MouseInside(&Button))
				ShowImage = i;

			static int s_NoneButton = 0;
			if(pEditor->DoButton_MenuItem(i == -1 ? (void *)&s_NoneButton : &pEditor->m_Map.m_vpImages[i], i == -1 ? "None" : pEditor->m_Map.m_vpImages[i]->m_aName, i == g_SelectImageCurrent, &Button))
				g_SelectImageSelected = i;
		}
	}

	s_ScrollRegion.End();

	if(ShowImage >= 0 && (size_t)ShowImage < pEditor->m_Map.m_vpImages.size())
	{
		if(ImageView.h < ImageView.w)
			ImageView.w = ImageView.h;
		else
			ImageView.h = ImageView.w;
		float Max = (float)(maximum(pEditor->m_Map.m_vpImages[ShowImage]->m_Width, pEditor->m_Map.m_vpImages[ShowImage]->m_Height));
		ImageView.w *= pEditor->m_Map.m_vpImages[ShowImage]->m_Width / Max;
		ImageView.h *= pEditor->m_Map.m_vpImages[ShowImage]->m_Height / Max;
		pEditor->Graphics()->TextureSet(pEditor->m_Map.m_vpImages[ShowImage]->m_Texture);
		pEditor->Graphics()->BlendNormal();
		pEditor->Graphics()->WrapClamp();
		pEditor->Graphics()->QuadsBegin();
		IGraphics::CQuadItem QuadItem(ImageView.x, ImageView.y, ImageView.w, ImageView.h);
		pEditor->Graphics()->QuadsDrawTL(&QuadItem, 1);
		pEditor->Graphics()->QuadsEnd();
		pEditor->Graphics()->WrapNormal();
	}

	return CUI::POPUP_KEEP_OPEN;
}

void CEditor::PopupSelectImageInvoke(int Current, float x, float y)
{
	static SPopupMenuId s_PopupSelectImageId;
	g_SelectImageSelected = -100;
	g_SelectImageCurrent = Current;
	UI()->DoPopupMenu(&s_PopupSelectImageId, x, y, 450, 300, this, PopupSelectImage);
}

int CEditor::PopupSelectImageResult()
{
	if(g_SelectImageSelected == -100)
		return -100;

	g_SelectImageCurrent = g_SelectImageSelected;
	g_SelectImageSelected = -100;
	return g_SelectImageCurrent;
}

static int g_SelectSoundSelected = -100;
static int g_SelectSoundCurrent = -100;

CUI::EPopupMenuFunctionResult CEditor::PopupSelectSound(void *pContext, CUIRect View, bool Active)
{
	CEditor *pEditor = static_cast<CEditor *>(pContext);

	const float ButtonHeight = 12.0f;
	const float ButtonMargin = 2.0f;

	static CScrollRegion s_ScrollRegion;
	vec2 ScrollOffset(0.0f, 0.0f);
	CScrollRegionParams ScrollParams;
	ScrollParams.m_ScrollbarWidth = 10.0f;
	ScrollParams.m_ScrollbarMargin = 3.0f;
	ScrollParams.m_ScrollUnit = (ButtonHeight + ButtonMargin) * 5;
	s_ScrollRegion.Begin(&View, &ScrollOffset, &ScrollParams);
	View.y += ScrollOffset.y;

	for(int i = -1; i < (int)pEditor->m_Map.m_vpSounds.size(); i++)
	{
		CUIRect Button;
		View.HSplitTop(ButtonMargin, nullptr, &View);
		View.HSplitTop(ButtonHeight, &Button, &View);
		if(s_ScrollRegion.AddRect(Button))
		{
			static int s_NoneButton = 0;
			if(pEditor->DoButton_MenuItem(i == -1 ? (void *)&s_NoneButton : &pEditor->m_Map.m_vpSounds[i], i == -1 ? "None" : pEditor->m_Map.m_vpSounds[i]->m_aName, i == g_SelectSoundCurrent, &Button))
				g_SelectSoundSelected = i;
		}
	}

	s_ScrollRegion.End();

	return CUI::POPUP_KEEP_OPEN;
}

void CEditor::PopupSelectSoundInvoke(int Current, float x, float y)
{
	static SPopupMenuId s_PopupSelectSoundId;
	g_SelectSoundSelected = -100;
	g_SelectSoundCurrent = Current;
	UI()->DoPopupMenu(&s_PopupSelectSoundId, x, y, 150, 300, this, PopupSelectSound);
}

int CEditor::PopupSelectSoundResult()
{
	if(g_SelectSoundSelected == -100)
		return -100;

	g_SelectSoundCurrent = g_SelectSoundSelected;
	g_SelectSoundSelected = -100;
	return g_SelectSoundCurrent;
}

static int s_GametileOpSelected = -1;

static const char *s_apGametileOpButtonNames[] = {
	"Air",
	"Hookable",
	"Death",
	"Unhookable",
	"Hookthrough",
	"Freeze",
	"Unfreeze",
	"Deep Freeze",
	"Deep Unfreeze",
	"Blue Check-Tele",
	"Red Check-Tele",
	"Live Freeze",
	"Live Unfreeze",
};

CUI::EPopupMenuFunctionResult CEditor::PopupSelectGametileOp(void *pContext, CUIRect View, bool Active)
{
	CEditor *pEditor = static_cast<CEditor *>(pContext);

	const int PreviousSelected = s_GametileOpSelected;

	CUIRect Button;
	for(size_t i = 0; i < std::size(s_apGametileOpButtonNames); ++i)
	{
		View.HSplitTop(2.0f, nullptr, &View);
		View.HSplitTop(12.0f, &Button, &View);
		if(pEditor->DoButton_Editor(&s_apGametileOpButtonNames[i], s_apGametileOpButtonNames[i], 0, &Button, 0, nullptr))
			s_GametileOpSelected = i;
	}

	return s_GametileOpSelected == PreviousSelected ? CUI::POPUP_KEEP_OPEN : CUI::POPUP_CLOSE_CURRENT;
}

void CEditor::PopupSelectGametileOpInvoke(float x, float y)
{
	static SPopupMenuId s_PopupSelectGametileOpId;
	s_GametileOpSelected = -1;
	UI()->DoPopupMenu(&s_PopupSelectGametileOpId, x, y, 120.0f, std::size(s_apGametileOpButtonNames) * 14.0f + 10.0f, this, PopupSelectGametileOp);
}

int CEditor::PopupSelectGameTileOpResult()
{
	if(s_GametileOpSelected < 0)
		return -1;

	int Result = s_GametileOpSelected;
	s_GametileOpSelected = -1;
	return Result;
}

static int s_AutoMapConfigSelected = -100;
static int s_AutoMapConfigCurrent = -100;

CUI::EPopupMenuFunctionResult CEditor::PopupSelectConfigAutoMap(void *pContext, CUIRect View, bool Active)
{
	CEditor *pEditor = static_cast<CEditor *>(pContext);
	std::shared_ptr<CLayerTiles> pLayer = std::static_pointer_cast<CLayerTiles>(pEditor->GetSelectedLayer(0));
	CAutoMapper *pAutoMapper = &pEditor->m_Map.m_vpImages[pLayer->m_Image]->m_AutoMapper;

	const float ButtonHeight = 12.0f;
	const float ButtonMargin = 2.0f;

	static CScrollRegion s_ScrollRegion;
	vec2 ScrollOffset(0.0f, 0.0f);
	CScrollRegionParams ScrollParams;
	ScrollParams.m_ScrollbarWidth = 10.0f;
	ScrollParams.m_ScrollbarMargin = 3.0f;
	ScrollParams.m_ScrollUnit = (ButtonHeight + ButtonMargin) * 5;
	s_ScrollRegion.Begin(&View, &ScrollOffset, &ScrollParams);
	View.y += ScrollOffset.y;

	for(int i = -1; i < pAutoMapper->ConfigNamesNum(); i++)
	{
		CUIRect Button;
		View.HSplitTop(ButtonMargin, nullptr, &View);
		View.HSplitTop(ButtonHeight, &Button, &View);
		if(s_ScrollRegion.AddRect(Button))
		{
			static int s_NoneButton = 0;
			if(pEditor->DoButton_MenuItem(i == -1 ? (void *)&s_NoneButton : pAutoMapper->GetConfigName(i), i == -1 ? "None" : pAutoMapper->GetConfigName(i), i == s_AutoMapConfigCurrent, &Button))
				s_AutoMapConfigSelected = i;
		}
	}

	s_ScrollRegion.End();

	return CUI::POPUP_KEEP_OPEN;
}

void CEditor::PopupSelectConfigAutoMapInvoke(int Current, float x, float y)
{
	static SPopupMenuId s_PopupSelectConfigAutoMapId;
	s_AutoMapConfigSelected = -100;
	s_AutoMapConfigCurrent = Current;
	std::shared_ptr<CLayerTiles> pLayer = std::static_pointer_cast<CLayerTiles>(GetSelectedLayer(0));
	const int ItemCount = minimum(m_Map.m_vpImages[pLayer->m_Image]->m_AutoMapper.ConfigNamesNum(), 10);
	// Width for buttons is 120, 15 is the scrollbar width, 2 is the margin between both.
	UI()->DoPopupMenu(&s_PopupSelectConfigAutoMapId, x, y, 120.0f + 15.0f + 2.0f, 26.0f + 14.0f * ItemCount, this, PopupSelectConfigAutoMap);
}

int CEditor::PopupSelectConfigAutoMapResult()
{
	if(s_AutoMapConfigSelected == -100)
		return -100;

	s_AutoMapConfigCurrent = s_AutoMapConfigSelected;
	s_AutoMapConfigSelected = -100;
	return s_AutoMapConfigCurrent;
}

// DDRace

CUI::EPopupMenuFunctionResult CEditor::PopupTele(void *pContext, CUIRect View, bool Active)
{
	CEditor *pEditor = static_cast<CEditor *>(pContext);

	static int s_PreviousTeleNumber;
	static int s_PreviousCheckpointNumber;
	static int s_PreviousViewTeleNumber;

	CUIRect NumberPicker;
	CUIRect FindEmptySlot;
	CUIRect FindFreeTeleSlot, FindFreeCheckpointSlot, FindFreeViewSlot;

	View.VSplitRight(15.f, &NumberPicker, &FindEmptySlot);
	NumberPicker.VSplitRight(2.f, &NumberPicker, nullptr);

	FindEmptySlot.HSplitTop(13.0f, &FindFreeTeleSlot, &FindEmptySlot);
	FindEmptySlot.HSplitTop(13.0f, &FindFreeCheckpointSlot, &FindEmptySlot);
	FindEmptySlot.HSplitTop(13.0f, &FindFreeViewSlot, &FindEmptySlot);

	FindFreeTeleSlot.HMargin(1.0f, &FindFreeTeleSlot);
	FindFreeCheckpointSlot.HMargin(1.0f, &FindFreeCheckpointSlot);
	FindFreeViewSlot.HMargin(1.0f, &FindFreeViewSlot);

	auto ViewTele = [](CEditor *pEd) -> bool {
		if(!pEd->m_ViewTeleNumber)
			return false;
		int TeleX, TeleY;
		pEd->m_Map.m_pTeleLayer->GetPos(pEd->m_ViewTeleNumber, -1, TeleX, TeleY);
		if(TeleX != -1 && TeleY != -1)
		{
			pEd->MapView()->SetWorldOffset({32.0f * TeleX + 0.5f, 32.0f * TeleY + 0.5f});
			return true;
		}
		return false;
	};

	static std::vector<ColorRGBA> s_vColors = {
		ColorRGBA(0.5f, 1, 0.5f, 0.5f),
		ColorRGBA(0.5f, 1, 0.5f, 0.5f),
		ColorRGBA(0.5f, 1, 0.5f, 0.5f),
	};
	enum
	{
		PROP_TELE = 0,
		PROP_TELE_CP,
		PROP_TELE_VIEW,
		NUM_PROPS,
	};

	// find next free numbers buttons
	{
		// Pressing ctrl+f will find next free numbers for both tele and checkpoints

		static int s_NextFreeTelePid = 0;
		if(pEditor->DoButton_Editor(&s_NextFreeTelePid, "F", 0, &FindFreeTeleSlot, 0, "[ctrl+f] Find next free tele number") || (Active && pEditor->Input()->ModifierIsPressed() && pEditor->Input()->KeyPress(KEY_F)))
		{
			int TeleNumber = pEditor->FindNextFreeTeleNumber();

			if(TeleNumber != -1)
				pEditor->m_TeleNumber = TeleNumber;
		}

		static int s_NextFreeCheckpointPid = 0;
		if(pEditor->DoButton_Editor(&s_NextFreeCheckpointPid, "F", 0, &FindFreeCheckpointSlot, 0, "[ctrl+f] Find next free checkpoint number") || (Active && pEditor->Input()->ModifierIsPressed() && pEditor->Input()->KeyPress(KEY_F)))
		{
			int CPNumber = pEditor->FindNextFreeTeleNumber(true);

			if(CPNumber != -1)
				pEditor->m_TeleCheckpointNumber = CPNumber;
		}

		static int s_NextFreeViewPid = 0;
		int btn = pEditor->DoButton_Editor(&s_NextFreeViewPid, "N", 0, &FindFreeViewSlot, 0, "[n] Show next tele with this number");
		if(btn || (Active && pEditor->Input()->KeyPress(KEY_N)))
			s_vColors[PROP_TELE_VIEW] = ViewTele(pEditor) ? ColorRGBA(0.5f, 1, 0.5f, 0.5f) : ColorRGBA(1, 0.5f, 0.5f, 0.5f);
	}

	// number picker
	{
		CProperty aProps[] = {
			{"Number", pEditor->m_TeleNumber, PROPTYPE_INT, 1, 255},
			{"Checkpoint", pEditor->m_TeleCheckpointNumber, PROPTYPE_INT, 1, 255},
			{"View", pEditor->m_ViewTeleNumber, PROPTYPE_INT, 1, 255},
			{nullptr},
		};

		static int s_aIds[NUM_PROPS] = {0};

		static int NewVal = 0;
		int Prop = pEditor->DoProperties(&NumberPicker, aProps, s_aIds, &NewVal, s_vColors);
		if(Prop == PROP_TELE)
			pEditor->m_TeleNumber = (NewVal - 1 + 255) % 255 + 1;
		else if(Prop == PROP_TELE_CP)
			pEditor->m_TeleCheckpointNumber = (NewVal - 1 + 255) % 255 + 1;
		else if(Prop == PROP_TELE_VIEW)
			pEditor->m_ViewTeleNumber = (NewVal - 1 + 255) % 255 + 1;

		if(s_PreviousTeleNumber == 1 || s_PreviousTeleNumber != pEditor->m_TeleNumber)
			s_vColors[PROP_TELE] = pEditor->m_Map.m_pTeleLayer->ContainsElementWithId(pEditor->m_TeleNumber, false) ? ColorRGBA(1, 0.5f, 0.5f, 0.5f) : ColorRGBA(0.5f, 1, 0.5f, 0.5f);

		if(s_PreviousCheckpointNumber == 1 || s_PreviousCheckpointNumber != pEditor->m_TeleCheckpointNumber)
			s_vColors[PROP_TELE_CP] = pEditor->m_Map.m_pTeleLayer->ContainsElementWithId(pEditor->m_TeleCheckpointNumber, true) ? ColorRGBA(1, 0.5f, 0.5f, 0.5f) : ColorRGBA(0.5f, 1, 0.5f, 0.5f);

		if(s_PreviousViewTeleNumber != pEditor->m_ViewTeleNumber)
			s_vColors[PROP_TELE_VIEW] = ViewTele(pEditor) ? ColorRGBA(0.5f, 1, 0.5f, 0.5f) : ColorRGBA(1, 0.5f, 0.5f, 0.5f);
	}

	s_PreviousTeleNumber = pEditor->m_TeleNumber;
	s_PreviousCheckpointNumber = pEditor->m_TeleCheckpointNumber;
	s_PreviousViewTeleNumber = pEditor->m_ViewTeleNumber;

	return CUI::POPUP_KEEP_OPEN;
}

CUI::EPopupMenuFunctionResult CEditor::PopupSpeedup(void *pContext, CUIRect View, bool Active)
{
	CEditor *pEditor = static_cast<CEditor *>(pContext);

	enum
	{
		PROP_FORCE = 0,
		PROP_MAXSPEED,
		PROP_ANGLE,
		NUM_PROPS
	};

	CProperty aProps[] = {
		{"Force", pEditor->m_SpeedupForce, PROPTYPE_INT, 1, 255},
		{"Max Speed", pEditor->m_SpeedupMaxSpeed, PROPTYPE_INT, 0, 255},
		{"Angle", pEditor->m_SpeedupAngle, PROPTYPE_ANGLE_SCROLL, 0, 359},
		{nullptr},
	};

	static int s_aIds[NUM_PROPS] = {0};
	int NewVal = 0;
	int Prop = pEditor->DoProperties(&View, aProps, s_aIds, &NewVal);

	if(Prop == PROP_FORCE)
	{
		pEditor->m_SpeedupForce = clamp(NewVal, 1, 255);
	}
	else if(Prop == PROP_MAXSPEED)
	{
		pEditor->m_SpeedupMaxSpeed = clamp(NewVal, 0, 255);
	}
	else if(Prop == PROP_ANGLE)
	{
		pEditor->m_SpeedupAngle = clamp(NewVal, 0, 359);
	}

	return CUI::POPUP_KEEP_OPEN;
}

CUI::EPopupMenuFunctionResult CEditor::PopupSwitch(void *pContext, CUIRect View, bool Active)
{
	CEditor *pEditor = static_cast<CEditor *>(pContext);

	CUIRect NumberPicker, FindEmptySlot;

	View.VSplitRight(15.0f, &NumberPicker, &FindEmptySlot);
	NumberPicker.VSplitRight(2.0f, &NumberPicker, nullptr);
	FindEmptySlot.HSplitTop(13.0f, &FindEmptySlot, nullptr);
	FindEmptySlot.HMargin(1.0f, &FindEmptySlot);

	// find empty number button
	{
		static int s_EmptySlotPid = 0;
		if(pEditor->DoButton_Editor(&s_EmptySlotPid, "F", 0, &FindEmptySlot, 0, "[ctrl+f] Find empty slot") || (Active && pEditor->Input()->ModifierIsPressed() && pEditor->Input()->KeyPress(KEY_F)))
		{
			int Number = pEditor->FindNextFreeSwitchNumber();

			if(Number != -1)
				pEditor->m_SwitchNum = Number;
		}
	}

	// number picker
	static int s_PreviousNumber = -1;
	{
		static std::vector<ColorRGBA> s_vColors = {
			ColorRGBA(1, 1, 1, 0.5f),
		};

		enum
		{
			PROP_SWITCH_NUMBER = 0,
			PROP_SWITCH_DELAY,
			NUM_PROPS,
		};

		CProperty aProps[] = {
			{"Number", pEditor->m_SwitchNum, PROPTYPE_INT, 0, 255},
			{"Delay", pEditor->m_SwitchDelay, PROPTYPE_INT, 0, 255},
			{nullptr},
		};

		static int s_aIds[NUM_PROPS] = {0};
		int NewVal = 0;
		int Prop = pEditor->DoProperties(&NumberPicker, aProps, s_aIds, &NewVal, s_vColors);

		if(Prop == PROP_SWITCH_NUMBER)
		{
			pEditor->m_SwitchNum = (NewVal + 256) % 256;
		}
		else if(Prop == PROP_SWITCH_DELAY)
		{
			pEditor->m_SwitchDelay = (NewVal + 256) % 256;
		}

		if(s_PreviousNumber == 1 || s_PreviousNumber != pEditor->m_SwitchNum)
			s_vColors[PROP_SWITCH_NUMBER] = pEditor->m_Map.m_pSwitchLayer->ContainsElementWithId(pEditor->m_SwitchNum) ? ColorRGBA(1, 0.5f, 0.5f, 0.5f) : ColorRGBA(0.5f, 1, 0.5f, 0.5f);
	}

	s_PreviousNumber = pEditor->m_SwitchNum;
	return CUI::POPUP_KEEP_OPEN;
}

CUI::EPopupMenuFunctionResult CEditor::PopupTune(void *pContext, CUIRect View, bool Active)
{
	CEditor *pEditor = static_cast<CEditor *>(pContext);

	enum
	{
		PROP_TUNE = 0,
		NUM_PROPS,
	};

	CProperty aProps[] = {
		{"Zone", pEditor->m_TuningNum, PROPTYPE_INT, 1, 255},
		{nullptr},
	};

	static int s_aIds[NUM_PROPS] = {0};
	int NewVal = 0;
	int Prop = pEditor->DoProperties(&View, aProps, s_aIds, &NewVal);

	if(Prop == PROP_TUNE)
	{
		pEditor->m_TuningNum = (NewVal - 1 + 255) % 255 + 1;
	}

	return CUI::POPUP_KEEP_OPEN;
}

CUI::EPopupMenuFunctionResult CEditor::PopupGoto(void *pContext, CUIRect View, bool Active)
{
	CEditor *pEditor = static_cast<CEditor *>(pContext);

	enum
	{
		PROP_COORD_X = 0,
		PROP_COORD_Y,
		NUM_PROPS,
	};

	static ivec2 s_GotoPos(0, 0);

	CProperty aProps[] = {
		{"X", s_GotoPos.x, PROPTYPE_INT, std::numeric_limits<int>::min(), std::numeric_limits<int>::max()},
		{"Y", s_GotoPos.y, PROPTYPE_INT, std::numeric_limits<int>::min(), std::numeric_limits<int>::max()},
		{nullptr},
	};

	static int s_aIds[NUM_PROPS] = {0};
	int NewVal = 0;
	int Prop = pEditor->DoProperties(&View, aProps, s_aIds, &NewVal);

	if(Prop == PROP_COORD_X)
	{
		s_GotoPos.x = NewVal;
	}
	else if(Prop == PROP_COORD_Y)
	{
		s_GotoPos.y = NewVal;
	}

	CUIRect Button;
	View.HSplitBottom(12.0f, &View, &Button);

	static int s_Button;
	if(pEditor->DoButton_Editor(&s_Button, "Go", 0, &Button, 0, nullptr))
	{
		pEditor->MapView()->SetWorldOffset({32.0f * s_GotoPos.x + 0.5f, 32.0f * s_GotoPos.y + 0.5f});
	}

	return CUI::POPUP_KEEP_OPEN;
}

CUI::EPopupMenuFunctionResult CEditor::PopupEntities(void *pContext, CUIRect View, bool Active)
{
	CEditor *pEditor = static_cast<CEditor *>(pContext);

	for(size_t i = 0; i < pEditor->m_vSelectEntitiesFiles.size(); i++)
	{
		CUIRect Button;
		View.HSplitTop(14.0f, &Button, &View);

		const char *pName = pEditor->m_vSelectEntitiesFiles[i].c_str();
		if(pEditor->DoButton_MenuItem(pName, pName, pEditor->m_vSelectEntitiesFiles[i] == pEditor->m_SelectEntitiesImage, &Button))
		{
			if(pEditor->m_vSelectEntitiesFiles[i] != pEditor->m_SelectEntitiesImage)
			{
				pEditor->m_SelectEntitiesImage = pEditor->m_vSelectEntitiesFiles[i];
				pEditor->m_AllowPlaceUnusedTiles = pEditor->m_SelectEntitiesImage == "DDNet" ? 0 : -1;
				pEditor->m_PreventUnusedTilesWasWarned = false;

				if(pEditor->m_EntitiesTexture.IsValid())
					pEditor->Graphics()->UnloadTexture(&pEditor->m_EntitiesTexture);

				char aBuf[IO_MAX_PATH_LENGTH];
				str_format(aBuf, sizeof(aBuf), "editor/entities/%s.png", pName);
				pEditor->m_EntitiesTexture = pEditor->Graphics()->LoadTexture(aBuf, IStorage::TYPE_ALL, pEditor->GetTextureUsageFlag());
				return CUI::POPUP_CLOSE_CURRENT;
			}
		}
	}

	return CUI::POPUP_KEEP_OPEN;
}

CUI::EPopupMenuFunctionResult CEditor::PopupProofMode(void *pContext, CUIRect View, bool Active)
{
	CEditor *pEditor = static_cast<CEditor *>(pContext);

	CUIRect Button;
	View.HSplitTop(12.0f, &Button, &View);
	static int s_ButtonIngame;
	if(pEditor->DoButton_MenuItem(&s_ButtonIngame, "Ingame", pEditor->MapView()->ProofMode()->IsModeIngame(), &Button, 0, "These borders represent what a player maximum can see."))
	{
		pEditor->MapView()->ProofMode()->SetModeIngame();
		return CUI::POPUP_CLOSE_CURRENT;
	}

	View.HSplitTop(2.0f, nullptr, &View);
	View.HSplitTop(12.0f, &Button, &View);
	static int s_ButtonMenu;
	if(pEditor->DoButton_MenuItem(&s_ButtonMenu, "Menu", pEditor->MapView()->ProofMode()->IsModeMenu(), &Button, 0, "These borders represent what will be shown in the menu."))
	{
		pEditor->MapView()->ProofMode()->SetModeMenu();
		return CUI::POPUP_CLOSE_CURRENT;
	}

	return CUI::POPUP_KEEP_OPEN;
}

CUI::EPopupMenuFunctionResult CEditor::PopupAnimateSettings(void *pContext, CUIRect View, bool Active)
{
	CEditor *pEditor = static_cast<CEditor *>(pContext);

	constexpr float MIN_ANIM_SPEED = 0.001f;
	constexpr float MAX_ANIM_SPEED = 1000000.0f;

	CUIRect Row, Label, ButtonDecrease, EditBox, ButtonIncrease, ButtonReset;
	View.HSplitTop(13.0f, &Row, &View);
	Row.VSplitMid(&Label, &Row);
	Row.HMargin(1.0f, &Row);
	Row.VSplitLeft(10.0f, &ButtonDecrease, &Row);
	Row.VSplitRight(10.0f, &EditBox, &ButtonIncrease);
	View.HSplitBottom(12.0f, &View, &ButtonReset);
	pEditor->UI()->DoLabel(&Label, "Speed", 10.0f, TEXTALIGN_ML);

	static char s_DecreaseButton;
	if(pEditor->DoButton_Ex(&s_DecreaseButton, "-", 0, &ButtonDecrease, 0, "Decrease animation speed", IGraphics::CORNER_L))
	{
		pEditor->m_AnimateSpeed -= pEditor->m_AnimateSpeed <= 1.0f ? 0.1f : 0.5f;
		pEditor->m_AnimateSpeed = maximum(pEditor->m_AnimateSpeed, MIN_ANIM_SPEED);
		pEditor->m_AnimateUpdatePopup = true;
	}

	static char s_IncreaseButton;
	if(pEditor->DoButton_Ex(&s_IncreaseButton, "+", 0, &ButtonIncrease, 0, "Increase animation speed", IGraphics::CORNER_R))
	{
		if(pEditor->m_AnimateSpeed < 0.1f)
			pEditor->m_AnimateSpeed = 0.1f;
		else
			pEditor->m_AnimateSpeed += pEditor->m_AnimateSpeed < 1.0f ? 0.1f : 0.5f;
		pEditor->m_AnimateSpeed = minimum(pEditor->m_AnimateSpeed, MAX_ANIM_SPEED);
		pEditor->m_AnimateUpdatePopup = true;
	}

	static char s_DefaultButton;
	if(pEditor->DoButton_Ex(&s_DefaultButton, "Default", 0, &ButtonReset, 0, "Normal animation speed", IGraphics::CORNER_ALL))
	{
		pEditor->m_AnimateSpeed = 1.0f;
		pEditor->m_AnimateUpdatePopup = true;
	}

	static CLineInputNumber s_SpeedInput;
	if(pEditor->m_AnimateUpdatePopup)
	{
		s_SpeedInput.SetFloat(pEditor->m_AnimateSpeed);
		pEditor->m_AnimateUpdatePopup = false;
	}

	if(pEditor->DoEditBox(&s_SpeedInput, &EditBox, 10.0f, IGraphics::CORNER_NONE, "The animation speed"))
	{
		pEditor->m_AnimateSpeed = clamp(s_SpeedInput.GetFloat(), MIN_ANIM_SPEED, MAX_ANIM_SPEED);
	}

	return CUI::POPUP_KEEP_OPEN;
}

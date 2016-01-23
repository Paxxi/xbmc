/*
 *      Copyright (C) 2005-2015 Team Kodi
 *      http://kodi.tv
 *
 *  This Program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *
 *  This Program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with Kodi; see the file COPYING.  If not, see
 *  <http://www.gnu.org/licenses/>.
 *
 */

#include "GUIAction.h"
#include "utils/StringUtils.h"
#include "GUIWindowManager.h"
#include "GUIControl.h"
#include "GUIInfoManager.h"

CGUIAction::CGUIAction()
{
  m_sendThreadMessages = false;
}

CGUIAction::CGUIAction(int controlID)
{
  m_sendThreadMessages = false;
  SetNavigation(controlID);
}

bool CGUIAction::ExecuteActions(int controlID, int parentID, const CGUIListItemPtr &item /* = NULL */) const
{
  if (m_actions.empty()) return false;
  // take a copy of actions that satisfy our conditions
  std::vector<std::string> actions;
  for (const auto & m_action : m_actions)
  {
    if (m_action.condition.empty() || g_infoManager.EvaluateBool(m_action.condition, 0, item))
    {
      if (!StringUtils::IsInteger(m_action.action))
        actions.push_back(m_action.action);
    }
  }
  // execute them
  bool retval = false;
  for (auto & action : actions)
  {
    CGUIMessage msg(GUI_MSG_EXECUTE, controlID, parentID, 0, 0, item);
    msg.SetStringParam(action);
    if (m_sendThreadMessages)
      g_windowManager.SendThreadMessage(msg);
    else
      g_windowManager.SendMessage(msg);
    retval = true;
  }
  return retval;
}

int CGUIAction::GetNavigation() const
{
  for (const auto & m_action : m_actions)
  {
    if (StringUtils::IsInteger(m_action.action))
    {
      if (m_action.condition.empty() || g_infoManager.EvaluateBool(m_action.condition))
        return atoi(m_action.action.c_str());
    }
  }
  return 0;
}

void CGUIAction::SetNavigation(int id)
{
  if (id == 0) return;
  std::string strId = StringUtils::Format("%i", id);
  for (auto & m_action : m_actions)
  {
    if (StringUtils::IsInteger(m_action.action) && m_action.condition.empty())
    {
      m_action.action = strId;
      return;
    }
  }
  cond_action_pair pair;
  pair.action = strId;
  m_actions.push_back(pair);
}

bool CGUIAction::HasActionsMeetingCondition() const
{
  for (const auto & m_action : m_actions)
  {
    if (m_action.condition.empty() || g_infoManager.EvaluateBool(m_action.condition))
      return true;
  }
  return false;
}

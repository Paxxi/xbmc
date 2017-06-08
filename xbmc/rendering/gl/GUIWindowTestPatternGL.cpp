/*
 *      Copyright (C) 2005-2013 Team XBMC
 *      http://xbmc.org
 *
 *      Test patterns designed by Ofer LaOr - hometheater.co.il
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
 *  along with XBMC; see the file COPYING.  If not, see
 *  <http://www.gnu.org/licenses/>.
 *
 */

#include "system.h"

#ifdef HAS_GL
#include "system_gl.h"
#include "GUIWindowTestPatternGL.h"

CGUIWindowTestPatternGL::CGUIWindowTestPatternGL() : CGUIWindowTestPattern()
{
}

CGUIWindowTestPatternGL::~CGUIWindowTestPatternGL(void)
{
}

void CGUIWindowTestPatternGL::DrawVerticalLines(int top, int left, int bottom, int right)
{
  glBegin(GL_LINES);
  glColor3f(m_white, m_white, m_white);
  for (int i = left; i <= right; i += 2)
  {
    glVertex2d(i, top);
    glVertex2d(i, bottom);
  }
  glEnd();
}

void CGUIWindowTestPatternGL::DrawHorizontalLines(int top, int left, int bottom, int right)
{
  glBegin(GL_LINES);
  glColor3f(m_white, m_white, m_white);
  for (int i = top; i <= bottom; i += 2)
  {
    glVertex2d(left, i);
    glVertex2d(right, i);
  }
  glEnd();
}

void CGUIWindowTestPatternGL::DrawCheckers(int top, int left, int bottom, int right)
{
  glBegin(GL_POINTS);
  glColor3f(m_white, m_white, m_white);
  for (int y = top; y <= bottom; y++)
  {
    for (int x = left; x <= right; x += 2)
    {
      if (y % 2 == 0) {
        glVertex2d(x, y);
      } else {
        glVertex2d(x+1, y);
}
    }
  }
  glEnd();
}

void CGUIWindowTestPatternGL::DrawBouncingRectangle(int top, int left, int bottom, int right)
{
  m_bounceX += m_bounceDirectionX;
  m_bounceY += m_bounceDirectionY;

  if ((m_bounceDirectionX == 1 && m_bounceX + TEST_PATTERNS_BOUNCE_SQUARE_SIZE == right) || (m_bounceDirectionX == -1 && m_bounceX == left)) {
    m_bounceDirectionX = -m_bounceDirectionX;
}

  if ((m_bounceDirectionY == 1 && m_bounceY + TEST_PATTERNS_BOUNCE_SQUARE_SIZE == bottom) || (m_bounceDirectionY == -1 && m_bounceY == top)) {
    m_bounceDirectionY = -m_bounceDirectionY;
}

  glColor3f(m_white, m_white, m_white);
  glRecti(m_bounceX, m_bounceY, m_bounceX + TEST_PATTERNS_BOUNCE_SQUARE_SIZE, m_bounceY + TEST_PATTERNS_BOUNCE_SQUARE_SIZE);
}

void CGUIWindowTestPatternGL::DrawContrastBrightnessPattern(int top, int left, int bottom, int right)
{
  int x5p = static_cast<int> (left + (0.05f * (right - left)));
  int y5p = static_cast<int> (top + (0.05f * (bottom - top)));
  int x12p = static_cast<int> (left + (0.125f * (right - left)));
  int y12p = static_cast<int> (top + (0.125f * (bottom - top)));
  int x25p = static_cast<int> (left + (0.25f * (right - left)));
  int y25p = static_cast<int> (top + (0.25f * (bottom - top)));
  int x37p = static_cast<int> (left + (0.375f * (right - left)));
  int y37p = static_cast<int> (top + (0.375f * (bottom - top)));
  int x50p = left + (right - left) / 2;
  int y50p = top + (bottom - top) / 2;
  int x62p = static_cast<int> (left + (0.625f * (right - left)));
  int y62p = static_cast<int> (top + (0.625f * (bottom - top)));
  int x75p = static_cast<int> (left + (0.75f * (right - left)));
  int y75p = static_cast<int> (top + (0.75f * (bottom - top)));
  int x87p = static_cast<int> (left + (0.875f * (right - left)));
  int y87p = static_cast<int> (top + (0.875f * (bottom - top)));
  int x95p = static_cast<int> (left + (0.95f * (right - left)));
  int y95p = static_cast<int> (top + (0.95f * (bottom - top)));

  m_blinkFrame = (m_blinkFrame + 1) % TEST_PATTERNS_BLINK_CYCLE;

  // draw main quadrants
  glColor3f(m_white, m_white, m_white);
  glRecti(x50p, top, right, y50p);
  glRecti(left, y50p, x50p, bottom);

  // draw border lines
  glBegin(GL_LINES);
    glColor3f(m_white, m_white, m_white);
    glVertex2d(left, y5p);
    glVertex2d(x50p, y5p);
    glVertex2d(x5p, top);
    glVertex2d(x5p, y50p);
    glVertex2d(x50p, y95p);
    glVertex2d(right, y95p);
    glVertex2d(x95p, y50p);
    glVertex2d(x95p, bottom);

    glColor3f(m_black, m_black, m_black);
    glVertex2d(x50p, y5p);
    glVertex2d(right, y5p);
    glVertex2d(x5p, y50p);
    glVertex2d(x5p, bottom);
    glVertex2d(left, y95p);
    glVertex2d(x50p, y95p);
    glVertex2d(x95p, top);
    glVertex2d(x95p, y50p);
  glEnd();

  // draw inner rectangles
  glColor3f(m_white, m_white, m_white);
  glRecti(x12p, y12p, x37p, y37p);
  glRecti(x62p, y62p, x87p, y87p);

  glColor3f(m_black, m_black, m_black);
  glRecti(x62p, y12p, x87p, y37p);
  glRecti(x12p, y62p, x37p, y87p);

  // draw inner circles
  if (m_blinkFrame < TEST_PATTERNS_BLINK_CYCLE / 2) {
    glColor3f(m_black + 0.05f, m_black + 0.05f, m_black + 0.05f);
  } else {
    glColor3f(0.0f, 0.0f, 0.0f); //BTB
}
  DrawCircle(x25p, y75p, (y37p - y12p) / 3);
  DrawCircle(x75p, y25p, (y37p - y12p) / 3);

  if (m_blinkFrame < TEST_PATTERNS_BLINK_CYCLE / 2) {
    glColor3f(m_white - 0.05f, m_white - 0.05f, m_white - 0.05f);
  } else {
    glColor3f(1.0f, 1.0f, 1.0f); //WTW
}
  DrawCircle(x25p, y25p, (y37p - y12p) / 3);
  DrawCircle(x75p, y75p, (y37p - y12p) / 3);
}

void CGUIWindowTestPatternGL::DrawCircle(int originX, int originY, int radius)
{
  float angle;
  int vectorX;
  int vectorY;
  int vectorY1 = originY;
  int vectorX1 = originX;

  glBegin(GL_TRIANGLES);
  for (int i = 0; i <= 360; i++)
  {
    angle = static_cast<float>((static_cast<double>(i))/57.29577957795135);
    vectorX = static_cast<int> (originX + (radius*static_cast<float>(sin(static_cast<double>(angle)))));
    vectorY = static_cast<int> (originY + (radius*static_cast<float>(cos(static_cast<double>(angle)))));
    glVertex2d(originX, originY);
    glVertex2d(vectorX1, vectorY1);
    glVertex2d(vectorX, vectorY);
    vectorY1 = vectorY;
    vectorX1 = vectorX;
  }
  glEnd();
}

void CGUIWindowTestPatternGL::BeginRender()
{
  glDisable(GL_TEXTURE_2D);
  glDisable(GL_BLEND);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void CGUIWindowTestPatternGL::EndRender()
{
  glEnable(GL_TEXTURE_2D);
}

#endif

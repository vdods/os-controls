#include "stdafx.h"
#include <Primitives.h>
#include "ExposeGroup.h"
#include "ExposeViewWindow.h"
#include "osinterface/OSWindow.h"
#include "osinterface/OSApp.h"

ExposeGroup::ExposeGroup() : m_center(Vector2::Zero()), m_minBounds(Vector2::Zero()), m_maxBounds(Vector2::Zero()) { }

void ExposeGroup::CalculateCenterAndBounds() {
  const int numGroupMembers = static_cast<int>(m_groupMembers.size());
  assert(numGroupMembers > 0);
  m_minBounds.setConstant(DBL_MAX);
  m_maxBounds.setConstant(-DBL_MAX);
  m_center.setZero();
  for (const std::shared_ptr<ExposeViewWindow>& window : m_groupMembers) {
    const Vector2 windowSize = window->GetOSSize();
    const Vector2 windowPos = window->GetOSPosition();// +0.5*windowSize;
    m_minBounds = m_minBounds.cwiseMin(windowPos - 0.5*windowSize);
    m_maxBounds = m_maxBounds.cwiseMax(windowPos + 0.5*windowSize);
    m_center += windowPos;
  }
  m_center /= numGroupMembers;
}

bool ExposeGroup::Intersects(const ExposeGroup& other) const {
  for (int d=0; d<2; d++) {
    if (other.m_maxBounds[d] < m_maxBounds[d] || other.m_minBounds[d] > m_maxBounds[d]) {
      return false;
    }
  }
  return true;
}

Vector2 ExposeGroup::MinMovementToResolveCollision(const ExposeGroup& other) const {
  const double diffX1 = other.m_maxBounds.x() - m_minBounds.x();
  const double diffX2 = m_maxBounds.x() - other.m_minBounds.x();
  const double diffY1 = other.m_maxBounds.y() - m_minBounds.y();
  const double diffY2 = m_maxBounds.y() - other.m_minBounds.y();

  double minX = diffX1;
  if (std::abs(diffX2) < std::abs(diffX1)) {
    minX = diffX2;
  }

  double minY = diffY1;
  if (std::abs(diffY2) < std::abs(diffY1)) {
    minY = diffY2;
  }

  Vector2 result(Vector2::Zero());
  if (std::abs(minX) < std::abs(minY)) {
    result.x() = minX;
  } else {
    result.y() = minY;
  }
  return result;
}

void ExposeGroup::Move(const Vector2& displacement) {
  m_center += displacement;
  m_minBounds += displacement;
  m_maxBounds += displacement;
}

#include "GuiGraphicsFlowComposition.h"

namespace vl
{
	namespace presentation
	{
		namespace compositions
		{
			using namespace collections;

/***********************************************************************
GuiFlowComposition
***********************************************************************/

			void GuiFlowComposition::UpdateFlowItemBounds()
			{
				auto clientMargin = axis->RealMarginToVirtualMargin(extraMargin);
				if (clientMargin.left < 0) clientMargin.left = 0;
				if (clientMargin.top < 0) clientMargin.top = 0;
				if (clientMargin.right < 0) clientMargin.right = 0;
				if (clientMargin.bottom < 0) clientMargin.bottom = 0;

				auto realFullSize = previousBounds.GetSize();
				auto clientSize = axis->RealSizeToVirtualSize(realFullSize);
				clientSize.x -= (clientMargin.left + clientMargin.right);
				clientSize.y -= (clientMargin.top + clientMargin.bottom);

				flowItemBounds.Resize(flowItems.Count());
				for (vint i = 0; i < flowItems.Count(); i++)
				{
					flowItemBounds[i] = Rect(Point(0, 0), flowItems[i]->GetMinSize());
				}

				vint currentIndex = 0;
				vint rowTop = 0;

				while (currentIndex < flowItems.Count())
				{
					auto itemSize = axis->RealSizeToVirtualSize(flowItemBounds[currentIndex].GetSize());
					vint rowWidth = itemSize.x;
					vint rowHeight = itemSize.y;
					vint rowItemCount = 1;

					for (vint i = currentIndex + 1; i < flowItems.Count(); i++)
					{
						itemSize = axis->RealSizeToVirtualSize(flowItemBounds[i].GetSize());
						vint itemWidth = itemSize.x + columnPadding;
						if (rowWidth + itemWidth > clientSize.x)
						{
							break;
						}
						rowWidth += itemWidth;
						if (rowHeight < itemSize.y)
						{
							rowHeight = itemSize.y;
						}
						rowItemCount++;
					}

					vint baseLine = 0;
					Array<vint> itemBaseLines(rowItemCount);
					for (vint i = 0; i < rowItemCount; i++)
					{
						vint index = currentIndex + i;
						vint itemBaseLine = 0;
						itemSize = axis->RealSizeToVirtualSize(flowItemBounds[index].GetSize());

						auto option = flowItems[index]->GetFlowOption();
						switch (option.baseline)
						{
						case GuiFlowOption::FromTop:
							itemBaseLine = option.distance;
							break;
						case GuiFlowOption::FromBottom:
							itemBaseLine = itemSize.y - option.distance;
							break;
						case GuiFlowOption::Percentage:
							itemBaseLine = (vint)(itemSize.y*option.percentage);
							break;
						}

						itemBaseLines[i] = itemBaseLine;
						if (baseLine < itemBaseLine)
						{
							baseLine = itemBaseLine;
						}
					}

					vint rowUsedWidth = 0;
					for (vint i = 0; i < rowItemCount; i++)
					{
						vint index = currentIndex + i;
						itemSize = axis->RealSizeToVirtualSize(flowItemBounds[index].GetSize());

						vint itemLeft = 0;
						vint itemTop = rowTop + baseLine - itemBaseLines[i];

						switch (alignment)
						{
						case FlowAlignment::Left:
							itemLeft = rowUsedWidth + i * columnPadding;
							break;
						case FlowAlignment::Center:
							itemLeft = rowUsedWidth + i * columnPadding + (clientSize.x - rowWidth) / 2;
							break;
						case FlowAlignment::Extend:
							if (i == 0)
							{
								itemLeft = rowUsedWidth;
							}
							else
							{
								itemLeft = rowUsedWidth + (vint)((double)(clientSize.x - rowWidth) * i / (rowItemCount - 1));
							}
							break;
						}

						flowItemBounds[index] = axis->VirtualRectToRealRect(
							realFullSize,
							Rect(
								Point(
									itemLeft + clientMargin.left,
									itemTop + clientMargin.top
									),
								itemSize
								)
							);
						rowUsedWidth += itemSize.x;
					}

					rowTop += rowHeight + rowPadding;
					currentIndex += rowItemCount;
				}
			}

			void GuiFlowComposition::OnBoundsChanged(GuiGraphicsComposition* sender, GuiEventArgs& arguments)
			{
				UpdateFlowItemBounds();
			}

			void GuiFlowComposition::OnChildInserted(GuiGraphicsComposition* child)
			{
				GuiBoundsComposition::OnChildInserted(child);
				auto item = dynamic_cast<GuiFlowItemComposition*>(child);
				if (item && !flowItems.Contains(item))
				{
					flowItems.Add(item);
					UpdateFlowItemBounds();
				}
			}

			void GuiFlowComposition::OnChildRemoved(GuiGraphicsComposition* child)
			{
				GuiBoundsComposition::OnChildRemoved(child);
				auto item=dynamic_cast<GuiFlowItemComposition*>(child);
				if(item)
				{
					flowItems.Remove(item);
					UpdateFlowItemBounds();
				}
			}

			GuiFlowComposition::GuiFlowComposition()
				:axis(new GuiDefaultAxis)
			{
				BoundsChanged.AttachMethod(this, &GuiFlowComposition::OnBoundsChanged);
			}

			GuiFlowComposition::~GuiFlowComposition()
			{
			}

			Margin GuiFlowComposition::GetExtraMargin()
			{
				return extraMargin;
			}

			void GuiFlowComposition::SetExtraMargin(Margin value)
			{
				extraMargin = value;
				UpdateFlowItemBounds();
			}

			vint GuiFlowComposition::GetRowPadding()
			{
				return rowPadding;
			}

			void GuiFlowComposition::SetRowPadding(vint value)
			{
				rowPadding = value;
				UpdateFlowItemBounds();
			}

			vint GuiFlowComposition::GetColumnPadding()
			{
				return columnPadding;
			}

			void GuiFlowComposition::SetColumnPadding(vint value)
			{
				columnPadding = value;
				UpdateFlowItemBounds();
			}

			Ptr<IGuiAxis> GuiFlowComposition::GetAxis()
			{
				return axis;
			}

			void GuiFlowComposition::SetAxis(Ptr<IGuiAxis> value)
			{
				if (value)
				{
					axis = value;
					UpdateFlowItemBounds();
				}
			}

			FlowAlignment GuiFlowComposition::GetAlignment()
			{
				return alignment;
			}

			void GuiFlowComposition::SetAlignment(FlowAlignment value)
			{
				alignment = value;
				UpdateFlowItemBounds();
			}
			
			Size GuiFlowComposition::GetMinPreferredClientSize()
			{
				Size minSize = GuiBoundsComposition::GetMinPreferredClientSize();
				if (GetMinSizeLimitation() == GuiGraphicsComposition::LimitToElementAndChildren)
				{
				}

				vint x = 0;
				vint y = 0;
				if (extraMargin.left > 0) x += extraMargin.left;
				if (extraMargin.right > 0) x += extraMargin.right;
				if (extraMargin.top > 0) y += extraMargin.top;
				if (extraMargin.bottom > 0) y += extraMargin.bottom;
				return minSize + Size(x, y);
			}

			Rect GuiFlowComposition::GetBounds()
			{
				for (vint i = 0; i < flowItems.Count(); i++)
				{
					if (flowItemBounds[i].GetSize() != flowItems[i]->GetMinSize())
					{
						UpdateFlowItemBounds();
						break;
					}
				}

				bounds = GuiBoundsComposition::GetBounds();
				return bounds;
			}

/***********************************************************************
GuiFlowItemComposition
***********************************************************************/

			void GuiFlowItemComposition::OnParentChanged(GuiGraphicsComposition* oldParent, GuiGraphicsComposition* newParent)
			{
				GuiGraphicsSite::OnParentChanged(oldParent, newParent);
				flowParent = newParent == 0 ? 0 : dynamic_cast<GuiFlowComposition*>(newParent);
			}

			Size GuiFlowItemComposition::GetMinSize()
			{
				return GetBoundsInternal(bounds).GetSize();
			}

			GuiFlowItemComposition::GuiFlowItemComposition()
			{
				SetMinSizeLimitation(GuiGraphicsComposition::LimitToElementAndChildren);
			}

			GuiFlowItemComposition::~GuiFlowItemComposition()
			{
			}
			
			bool GuiFlowItemComposition::IsSizeAffectParent()
			{
				return false;
			}

			Rect GuiFlowItemComposition::GetBounds()
			{
				Rect result = bounds;
				if(flowParent)
				{
					vint index = flowParent->flowItems.IndexOf(this);
					if (index != -1)
					{
						result = flowParent->flowItemBounds[index];
					}

					result = Rect(
						result.Left() - extraMargin.left,
						result.Top() - extraMargin.top,
						result.Right() + extraMargin.right,
						result.Bottom() + extraMargin.bottom
						);
				}
				UpdatePreviousBounds(result);
				return result;
			}

			void GuiFlowItemComposition::SetBounds(Rect value)
			{
				bounds = value;
			}

			Margin GuiFlowItemComposition::GetExtraMargin()
			{
				return extraMargin;
			}

			void GuiFlowItemComposition::SetExtraMargin(Margin value)
			{
				extraMargin = value;
			}

			GuiFlowOption GuiFlowItemComposition::GetFlowOption()
			{
				return option;
			}

			void GuiFlowItemComposition::SetFlowOption(GuiFlowOption value)
			{
				option = value;
				if (flowParent)
				{
					flowParent->UpdateFlowItemBounds();
				}
			}
		}
	}
}
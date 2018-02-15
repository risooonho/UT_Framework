/************************************************************************/
/* UMBRA TOOLS                                                          */
/* Maxwell - Axel Clerget   											*/
/************************************************************************/

#include "NavigableVerticalBox.h"
#include "NavigableWidget.h"
#include "Kismet/GameplayStatics.h"
#include "NavigableWidgetLibrary.h"

#define LOCTEXT_NAMESPACE "UmbraFramework"

UNavigableVerticalBox::UNavigableVerticalBox(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UNavigableVerticalBox::OnSlotAdded(UPanelSlot* Slot)
{
	if (UNavigableWidget* Widget = Cast<UNavigableWidget>(Slot->Content))
	{
		this->NavigableWidgets.Add(Widget);
		Widget->OwnerContainer = this;
	}

	Super::OnSlotAdded(Slot);
}

void UNavigableVerticalBox::OnSlotRemoved(UPanelSlot* Slot)
{
	if (UNavigableWidget* Widget = Cast<UNavigableWidget>(Slot->Content))
	{
		this->NavigableWidgets.Remove(Widget);
		Widget->OwnerContainer = nullptr;
	}

	Super::OnSlotRemoved(Slot);
}

void UNavigableVerticalBox::BindInputs()
{
	if (this->InputComponent)
	{
		for (FInputActionKeyMapping KeyMapping : this->NavigateNextKeys)
		{
			this->InputComponent->BindKey(KeyMapping.Key, EInputEvent::IE_Pressed, this, &UNavigableVerticalBox::HandleNextKeyPressed);
			this->InputComponent->BindKey(KeyMapping.Key, EInputEvent::IE_Repeat, this, &UNavigableVerticalBox::HandleNextKeyPressed);
		}

		for (FInputActionKeyMapping KeyMapping : this->NavigatePreviousKeys)
		{
			this->InputComponent->BindKey(KeyMapping.Key, EInputEvent::IE_Pressed, this, &UNavigableVerticalBox::HandlePreviousKeyPressed);
			this->InputComponent->BindKey(KeyMapping.Key, EInputEvent::IE_Repeat, this, &UNavigableVerticalBox::HandlePreviousKeyPressed);
		}

		for (FInputActionKeyMapping KeyMapping : this->Confirmkeys)
		{
			this->InputComponent->BindKey(KeyMapping.Key, EInputEvent::IE_Pressed, this, &UNavigableVerticalBox::HandleConfirmKeyPressed);
		}
	}
}

void UNavigableVerticalBox::UnBindInputs()
{
	if (this->InputComponent)
		this->InputComponent->ClearActionBindings();
}

UNavigableWidget* UNavigableVerticalBox::GetFocusedNavigationWidget()
{
	for (UNavigableWidget* NavigableWidget : this->NavigableWidgets)
	{
		if (NavigableWidget->IsFocused)
			return NavigableWidget;
	}

	return nullptr;
}

UNavigableWidget* UNavigableVerticalBox::GetFirstNavigableWidget()
{
	return this->NavigableWidgets[0];
}

APlayerController* UNavigableVerticalBox::GetOwningPlayerController()
{
	return UGameplayStatics::GetPlayerController(this, 0);
}

bool UNavigableVerticalBox::ContainNavigableWidget()
{
	return this->NavigableWidgets.Num() > 0;
}

void UNavigableVerticalBox::HandleNextKeyPressed()
{
	if (this->HasConfirmed)
		return;

	for (int32 Index = 0; Index < this->NavigableWidgets.Num(); Index++)
	{
		if (this->NavigableWidgets[Index]->IsFocused && this->NavigableWidgets.IsValidIndex(Index + 1))
		{
			UNavigableWidgetLibrary::FocusNavigableWidget(this, this->NavigableWidgets[Index + 1]);
			return;
		}
	}

	if (this->BoundOpposite)
		UNavigableWidgetLibrary::FocusNavigableWidget(this, this->NavigableWidgets[0]);
}

void UNavigableVerticalBox::HandlePreviousKeyPressed()
{
	if (this->HasConfirmed)
		return;

	for (int32 Index = 0; Index < this->NavigableWidgets.Num(); Index++)
	{
		if (this->NavigableWidgets[Index]->IsFocused && this->NavigableWidgets.IsValidIndex(Index - 1))
		{
			UNavigableWidgetLibrary::FocusNavigableWidget(this, this->NavigableWidgets[Index -1]);
			return;
		}
	}

	if (this->BoundOpposite)
		UNavigableWidgetLibrary::FocusNavigableWidget(this, this->NavigableWidgets[this->NavigableWidgets.Num() - 1]);
}

void UNavigableVerticalBox::HandleConfirmKeyPressed()
{
	if (this->GetFocusedNavigationWidget())
	{
		this->HasConfirmed = true;
		this->GetFocusedNavigationWidget()->ConfirmState();
	}
}

void UNavigableVerticalBox::UnFocusAllNavigableWidget()
{
	for (auto const& Widget : this->NavigableWidgets)
	{
		Widget->LoseFocus();
	}
}

#if WITH_EDITOR

const FText UNavigableVerticalBox::GetPaletteCategory()
{
	return LOCTEXT("UmbraFramework", "Umbra Framework");
}

#endif

#undef LOCTEXT_NAMESPACE
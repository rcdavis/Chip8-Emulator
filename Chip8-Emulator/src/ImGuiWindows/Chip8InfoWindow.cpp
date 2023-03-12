#include "Chip8InfoWindow.h"

#include "Log.h"
#include "Chip8.h"

#include <imgui.h>

Chip8InfoWindow::Chip8InfoWindow(bool isOpen) :
    ImGuiWindow("Chip8 Info", "chip8InfoWindowOpen", isOpen)
{}

void Chip8InfoWindow::OnRender()
{
    if (!mChip8)
    {
        ImGui::Text("Chip8 pointer is NULL!");
        return;
    }

    ImVec4 drawnColor = ImGui::ColorConvertU32ToFloat4(mChip8->GetDrawnColor());
    ImGui::ColorEdit3("Drawn", &drawnColor.x);
    mChip8->SetDrawnColor(ImGui::ColorConvertFloat4ToU32(drawnColor));

    ImVec4 undrawnColor = ImGui::ColorConvertU32ToFloat4(mChip8->GetUndrawnColor());
    ImGui::ColorEdit3("Undrawn", &undrawnColor.x);
    mChip8->SetUndrawnColor(ImGui::ColorConvertFloat4ToU32(undrawnColor));

    int emuSpeed = (int)mChip8->GetEmuSpeedModifier();
    ImGui::SliderInt("Emu Speed Modifier", &emuSpeed, 1, 10);
    mChip8->SetEmuSpeedModifier((uint8_t)emuSpeed);

    ImGui::Separator();

    ImGui::Text("Quirk Flags");

    bool useVYShiftQuirk = mChip8->GetUseVYForShiftQuirk();
    ImGui::Checkbox("Use VY For Shift", &useVYShiftQuirk);
    mChip8->SetUseVYForShiftQuirk(useVYShiftQuirk);

    bool useBXNN = mChip8->GetUseBXNNQuirk();
    ImGui::Checkbox("Use BXNN", &useBXNN);
    mChip8->SetUseBXNNQuirk(useBXNN);

    bool useIndexIncrement = mChip8->GetUseIndexIncrementAfterStoreLoadQuirk();
    ImGui::Checkbox("Use Index Increment After Store and Load Memory", &useIndexIncrement);
    mChip8->SetUseIndexIncrementAfterStoreLoadQuirk(useIndexIncrement);

    ImGui::Separator();

    ImGui::Text("Opcode: %X", mChip8->GetOpcode());
    ImGui::Text("Index Reg: %X", mChip8->GetIndexReg());
    ImGui::Text("Program Counter: %X", mChip8->GetProgramCounter());
    ImGui::Text("Stack Pointer: %d", mChip8->GetStackPointer());
    ImGui::Text("Delay Timer: %d", mChip8->GetDelayTimer());
    ImGui::Text("Sound Timer: %d", mChip8->GetSoundTimer());

    ImGui::Separator();

    const auto vreg = mChip8->GetVReg();
    for (size_t i = 0; i < std::size(vreg); ++i)
        ImGui::Text("V%X: %d", i, vreg[i]);

    ImGui::Separator();

    const auto& keys = mChip8->GetKeys();
    for (size_t i = 0; i < std::size(keys); ++i)
        ImGui::Text("Key[%X] = %d", i, keys[i]);
}

/* Quick Menu checkbox that turns the overlay on and off. A QuickMenuReborn
 * plugin: QuickMenuReborn loads every .suprx in its folder into SceShell a few
 * seconds after boot, and builds the registered widgets each time the Quick
 * Menu opens. The switch itself is the shell plugin's, in the same process. */
#include <psp2/kernel/modulemgr.h>
#include <quickmenureborn/qm_reborn.h>

#include "../include/vjo_api.h"

#define SEPARATOR_ID "vita_jp_overlay_separator"
#define PLANE_ID     "vita_jp_overlay_plane"
#define ICON_ID      "vita_jp_overlay_icon"
#define TEXTURE_ID   "vita_jp_overlay_texture"
#define LABEL_ID     "vita_jp_overlay_label"
#define CHECKBOX_ID  "vita_jp_overlay_checkbox"
#define ICON_PATH    "ur0:QuickMenuReborn/VitaJPOverlay_Widget.png"

BUTTON_HANDLER(on_toggle)
{
    vjoShellSetEnabled(QuickMenuRebornGetCheckboxValue(CHECKBOX_ID) > 0);
}

void _start() __attribute__((weak, alias("module_start")));
int module_start(SceSize argc, const void *args)
{
    QuickMenuRebornSeparator(SEPARATOR_ID, SCE_SEPARATOR_HEIGHT);

    QuickMenuRebornRegisterWidget(PLANE_ID, NULL, plane);
    QuickMenuRebornSetWidgetSize(PLANE_ID, SCE_PLANE_WIDTH, 100, 0, 0);
    QuickMenuRebornSetWidgetColor(PLANE_ID, 1, 1, 1, 0);

    QuickMenuRebornRegisterTexture(TEXTURE_ID, ICON_PATH);
    QuickMenuRebornRegisterWidget(ICON_ID, PLANE_ID, plane);
    QuickMenuRebornSetWidgetSize(ICON_ID, 64, 64, 0, 0);
    QuickMenuRebornSetWidgetPosition(ICON_ID, -350, 0, 0, 0);
    QuickMenuRebornSetWidgetColor(ICON_ID, 1, 1, 1, 1);
    QuickMenuRebornSetWidgetTexture(ICON_ID, TEXTURE_ID);

    QuickMenuRebornRegisterWidget(LABEL_ID, PLANE_ID, text);
    QuickMenuRebornSetWidgetSize(LABEL_ID, 500, 75, 0, 0);
    QuickMenuRebornSetWidgetPosition(LABEL_ID, -220, 0, 0, 0);
    QuickMenuRebornSetWidgetColor(LABEL_ID, 1, 1, 1, 1);
    QuickMenuRebornSetWidgetLabel(LABEL_ID, "Vita JP Overlay");

    QuickMenuRebornRegisterWidget(CHECKBOX_ID, PLANE_ID, check_box);
    QuickMenuRebornSetWidgetSize(CHECKBOX_ID, 48, 48, 0, 0);
    QuickMenuRebornSetWidgetPosition(CHECKBOX_ID, 350, 0, 0, 0);
    QuickMenuRebornSetWidgetColor(CHECKBOX_ID, 1, 1, 1, 1);
    /* QuickMenuReborn keeps the tick in a file between Quick Menu openings.
     * The overlay is on after every boot, so last boot's tick is stale. */
    QuickMenuRebornSaveCheckBoxState(CHECKBOX_ID, 1);
    QuickMenuRebornAssignDefaultCheckBoxRecall(CHECKBOX_ID);
    QuickMenuRebornAssignDefaultCheckBoxSave(CHECKBOX_ID);
    QuickMenuRebornRegisterEventHanlder(CHECKBOX_ID, QMR_BUTTON_RELEASE_ID, on_toggle, NULL);

    return SCE_KERNEL_START_SUCCESS;
}

int module_stop(SceSize argc, const void *args)
{
    QuickMenuRebornUnregisterWidget(CHECKBOX_ID);
    QuickMenuRebornUnregisterWidget(LABEL_ID);
    QuickMenuRebornUnregisterWidget(ICON_ID);
    QuickMenuRebornUnregisterTexture(TEXTURE_ID);
    QuickMenuRebornUnregisterWidget(PLANE_ID);
    QuickMenuRebornRemoveSeparator(SEPARATOR_ID);
    return SCE_KERNEL_STOP_SUCCESS;
}

#include "assets_icons.h"
#include "dolphin/helpers/dolphin_state.h"
#include <core/check.h>
#include <core/record.h>
#include <furi.h>
#include <gui/gui.h>
#include <furi_hal_version.h>
#include "dolphin/dolphin.h"
#include "math.h"

#define MOODS_PORTRAITS_TOTAL 3

static const Icon* const portrait_happy[MOODS_PORTRAITS_TOTAL] = {
    &I_passport_happy1_46x49,
    &I_passport_happy2_46x49,
    &I_passport_happy3_46x49};
static const Icon* const portrait_ok[MOODS_PORTRAITS_TOTAL] = {
    &I_passport_okay1_46x49,
    &I_passport_okay2_46x49,
    &I_passport_okay3_46x49};
static const Icon* const portrait_bad[MOODS_PORTRAITS_TOTAL] = {
    &I_passport_bad1_46x49,
    &I_passport_bad2_46x49,
    &I_passport_bad3_46x49};

static const Icon* const* portraits[MOODS_PORTRAITS_TOTAL] = {
    portrait_happy,
    portrait_ok,
    portrait_bad};

static const char* const moods[BUTTHURT_MAX] = {
    "Joyful",
    "Happy",
    "Satisfied",
    "Good",
    "Relaxed",
    "Okay",
    "Tired",
    "Bored",
    "Sad",
    "Disappointed",
    "Annoyed",
    "Upset",
    "Angry",
    "Furious"};

static void input_callback(InputEvent* input, void* ctx) {
    FuriSemaphore* semaphore = ctx;

    if((input->type == InputTypeShort) && (input->key == InputKeyBack)) {
        furi_semaphore_release(semaphore);
    }
}

static void render_callback(Canvas* canvas, void* ctx) {
    DolphinStats* stats = ctx;

    char level_str[12];
    char xp_str[12];
    char mood_str[20];
    uint8_t mood = stats->butthurt / 5;
    snprintf(mood_str, 20, "Mood: %s", moods[stats->butthurt]);

    uint32_t xp_progress = 0;
    uint32_t xp_to_levelup = dolphin_state_xp_to_levelup(stats->icounter);
    uint32_t xp_above_last_levelup = dolphin_state_xp_above_last_levelup(stats->icounter);
    uint32_t xp_for_current_level = xp_to_levelup + xp_above_last_levelup;
    if(stats->level == LEVEL_MAX) {
        xp_progress = 0;
    } else {
        xp_progress = xp_to_levelup * 64 / xp_for_current_level;
    }

    // multipass
    canvas_draw_icon(canvas, 0, 0, &I_passport_left_6x46);
    canvas_draw_icon(canvas, 0, 46, &I_passport_bottom_128x18);
    canvas_draw_line(canvas, 6, 0, 125, 0);
    canvas_draw_line(canvas, 127, 2, 127, 47);
    canvas_draw_dot(canvas, 126, 1);

    // portrait
    furi_assert((stats->level > 0) && (stats->level <= LEVEL_MAX));
    uint16_t tmpLvl = 0;
    if(stats->level >= STAGE2_FORM_THRESHOLD) tmpLvl = 1;
    if(stats->level >= STAGE3_FORM_THRESHOLD) tmpLvl = 2;
    canvas_draw_icon(canvas, 9, 5, portraits[mood][tmpLvl]);
    canvas_draw_line(canvas, 58, 14, 123, 14);
    canvas_draw_line(canvas, 58, 26, 123, 26);
    canvas_draw_line(canvas, 58, 46, 123, 46);

    const char* my_name_ptr = furi_hal_version_get_name_ptr();
    const char* my_name = my_name_ptr ? my_name_ptr : "Unknown";
    FuriHalVersionGender my_gender = furi_hal_version_get_gender();
    snprintf(level_str, 12, "Level: %hu", stats->level);
    snprintf(xp_str, 12, "%lu/%lu", xp_above_last_levelup, xp_for_current_level);
    canvas_set_font(canvas, FontSecondary);
    canvas_draw_str(canvas, 58, 12, my_name);
    if(my_gender != FuriHalVersionGenderUnknown) {
        const Icon* gender_icon = my_gender == FuriHalVersionGenderMale ?
                                      &I_passport_gendermale_5x9 :
                                      &I_passport_genderfemale_5x9;
        canvas_draw_icon(canvas, 119, 4, gender_icon);
    }
    canvas_draw_str(canvas, 58, 24, mood_str);
    canvas_set_color(canvas, ColorBlack);
    canvas_draw_str(canvas, 58, 36, level_str);
    canvas_set_font(canvas, FontBatteryPercent);
    canvas_draw_str(canvas, 58, 44, xp_str);
    canvas_set_font(canvas, FontSecondary);

    canvas_set_color(canvas, ColorWhite);
    canvas_draw_box(canvas, 123 - xp_progress, 50, xp_progress + 1, 3);
    canvas_set_color(canvas, ColorBlack);
    canvas_draw_line(canvas, 123, 50, 123, 52);
}

int32_t passport_app(void* p) {
    UNUSED(p);
    FuriSemaphore* semaphore = furi_semaphore_alloc(1, 0);
    furi_assert(semaphore);

    ViewPort* view_port = view_port_alloc();

    Dolphin* dolphin = furi_record_open(RECORD_DOLPHIN);
    DolphinStats stats = dolphin_stats(dolphin);
    furi_record_close(RECORD_DOLPHIN);
    view_port_draw_callback_set(view_port, render_callback, &stats);
    view_port_input_callback_set(view_port, input_callback, semaphore);
    Gui* gui = furi_record_open(RECORD_GUI);
    gui_add_view_port(gui, view_port, GuiLayerFullscreen);
    view_port_update(view_port);

    furi_check(furi_semaphore_acquire(semaphore, FuriWaitForever) == FuriStatusOk);

    gui_remove_view_port(gui, view_port);
    view_port_free(view_port);
    furi_record_close(RECORD_GUI);
    furi_semaphore_free(semaphore);

    return 0;
}

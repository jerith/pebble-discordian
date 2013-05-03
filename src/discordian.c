#include "pebble_os.h"
#include "pebble_app.h"
#include "pebble_fonts.h"


#define MY_UUID { 0x62, 0x02, 0xC5, 0x1D, 0x0E, 0x7F, 0x48, 0xDC, 0xA5, 0xA3, 0x44, 0xAF, 0x43, 0x80, 0x27, 0x8D }
PBL_APP_INFO(MY_UUID,
             "Discordian", "jerith",
             1, 0, /* App version */
             DEFAULT_MENU_ICON,
             APP_INFO_WATCH_FACE);

Window window;

TextLayer text_ddate_layer;
TextLayer text_time_layer;
TextLayer text_gdate_layer;


#define DD_BG_COLOR GColorBlack
#define DD_FG_COLOR GColorWhite


void init_text_layer(TextLayer *layer, int16_t top, uint32_t font_res_id) {
    // We draw all layers starting at the `top' x co-ord and filling the rest of the screen.
    text_layer_init(layer, window.layer.frame);
    text_layer_set_text_color(layer, DD_FG_COLOR);
    text_layer_set_background_color(layer, GColorClear);
    layer_set_frame(&layer->layer, GRect(0, top, 144, 168 - top));
    text_layer_set_text_alignment(layer, GTextAlignmentCenter);
    text_layer_set_font(layer, fonts_load_custom_font(resource_get_handle(font_res_id)));
    layer_add_child(&window.layer, &layer->layer);
}


#define dseason( x ) ((x) == 0 ? "Chaos" :       \
                     (x) == 1 ? "Discord" :     \
                     (x) == 2 ? "Confusion" :   \
                     (x) == 3 ? "Bureaucracy" : \
                     "The Aftermath")

#define dweekday( x ) ((x) == 1 ? "Sweetmorn" :       \
                       (x) == 2 ? "Boomtime" :        \
                       (x) == 3 ? "Pungenday" :       \
                       (x) == 4 ? "Prickle-Prickle" : \
                       "Setting Orange")

#define ddate( x ) ((x)%73 == 0 ? 73 : (x)%73)

#define leap_year( x ) ((x) % 400 == 0 || (((x) % 4) == 0 && (x) % 100))


/*
 * We can't use sprintf() to build the Discordian date string because it isn't
 * completely implemented. To get around this, we have a custom function to
 * build a decimal string representation of an integer (maximum four digits)
 * and use strcat() to glue everything together.
 */

char *int_to_str(int16_t num) {
    // This is from a Discordian date, so we have a maximum of four digits for
    // the next six and a half thousand years.
    static char text_num[] = "0000";

    for (int i = 3; i >= 0; i--) {
        text_num[i] = '0' + (num % 10);
        num /= 10;
        if (num == 0) {
            // Return from the middle of our string.
            return text_num + i;
        }
    }
    return text_num;
}


void mk_discordian_date(char* ddate_text, PblTm *tick_time) {
    int dyear = tick_time->tm_year + 1900 + 1166;
    int dday = tick_time->tm_yday + 1;

    ddate_text[0] = '\0';

    if (leap_year(tick_time->tm_year)) {
        if (dday == 60) {
            // Ideally, we'd use sprintf() instead of all the strcat()ing.
            /* sprintf(ddate_text, "St. Tib's Day\n-------------\n%d YOLD", dyear); */
            strcat(ddate_text, "St. Tib's Day\n-------------\n");
            strcat(ddate_text, int_to_str(dyear));
            strcat(ddate_text, " YOLD");
            return;
        } else if (dday > 60) {
            --dday;
        }
    }

    // Ideally, we'd use sprintf() instead of all the strcat()ing.
    /* sprintf(ddate_text, "%s\n%s %d\n%d YOLD", */
    /*         dweekday(dday % 5), dseason(dday / 73), ddate(dday), dyear); */

    strcat(ddate_text, dweekday(dday % 5));
    strcat(ddate_text, "\n");
    strcat(ddate_text, dseason(dday / 73));
    strcat(ddate_text, " ");
    strcat(ddate_text, int_to_str(ddate(dday)));
    strcat(ddate_text, "\n");
    strcat(ddate_text, int_to_str(dyear));
    strcat(ddate_text, " YOLD");

};


void mk_gregorian_date(char* gdate_text, PblTm *tick_time) {
    char date_buf[] = "99 Mmm";

    string_format_time(gdate_text, sizeof(gdate_text), "%a", tick_time);
    strcat(gdate_text, " ");

    string_format_time(date_buf, sizeof(date_buf), "%e %b", tick_time);
    if (date_buf[0] == ' ' || date_buf[0] == '0') {
        strcat(gdate_text, &date_buf[1]);
    } else {
        strcat(gdate_text, date_buf);
    }
};


void display_time(PblTm *tick_time) {
    // Static, because we pass them to the system.
    static char time_text[] = "99:99";
    static char gdate_text[] = "Ddd 99 Mmm";
    static char ddate_text[] = "Prickle-Prickle\nThe Aftermath 39\n3179 YOLD";

    char *time_format;

    // Calculate time.

    if (clock_is_24h_style()) {
        time_format = "%H:%M";
    } else {
        time_format = "%I:%M";
    }

    string_format_time(time_text, sizeof(time_text), time_format, tick_time);

    // Kludge to handle lack of non-padded hour format string
    // for twelve hour clock.
    if (!clock_is_24h_style() && ((time_text[0] == '0') || time_text[0] == ' ')) {
        memmove(time_text, &time_text[1], sizeof(time_text) - 1);
    }

    // Calculate dates.

    mk_gregorian_date(gdate_text, tick_time);
    mk_discordian_date(ddate_text, tick_time);

    text_layer_set_text(&text_time_layer, time_text);
    text_layer_set_text(&text_gdate_layer, gdate_text);
    text_layer_set_text(&text_ddate_layer, ddate_text);
}


void handle_init(AppContextRef ctx) {
    PblTm init_time;

    (void) ctx;

    window_init(&window, "Discordian");
    window_stack_push(&window, true /* Animated */);
    window_set_background_color(&window, GColorBlack);

    resource_init_current_app(&APP_RESOURCES);

    init_text_layer(&text_ddate_layer, 4, RESOURCE_ID_FONT_DISCORDIAN_DATE_19);
    init_text_layer(&text_time_layer, 65, RESOURCE_ID_FONT_TIME_49);
    init_text_layer(&text_gdate_layer, 135, RESOURCE_ID_FONT_GREGORIAN_DATE_19);

    get_time(&init_time);
    display_time(&init_time);
}


void handle_minute_tick(AppContextRef ctx, PebbleTickEvent *t) {
    (void) ctx;

    display_time(t->tick_time);
}


void pbl_main(void *params) {
    PebbleAppHandlers handlers = {
        .init_handler = &handle_init,
        .tick_info = {
            .tick_handler = &handle_minute_tick,
            .tick_units = MINUTE_UNIT
        }
    };
    app_event_loop(params, &handlers);
}

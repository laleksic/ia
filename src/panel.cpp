#include "panel.hpp"

#include "io.hpp"
#include "rl_utils.hpp"

// -----------------------------------------------------------------------------
// Private
// -----------------------------------------------------------------------------
static R panels_[(size_t)Panel::END];

static bool is_valid_;

static void set_panel(
        const Panel panel,
        const int x0,
        const int y0,
        const int x1,
        const int y1)
{
        panels_[(size_t)panel] = {x0, y0, x1, y1};
}

static void set_x0(const Panel panel, int x)
{
        R& r =  panels_[(size_t)panel];

        const int w = r.w();

        r.p0.x = x;
        r.p1.x = x + w - 1;
}

static void set_y0(const Panel panel, int y)
{
        R& r =  panels_[(size_t)panel];

        const int h = r.h();

        r.p0.y = y;
        r.p1.y = y + h - 1;
}

static void set_w(const Panel panel, int w)
{
        R& r =  panels_[(size_t)panel];

        r.p1.x = r.p0.x + w - 1;
}

static void finalize_screen_dims()
{
        R& screen = panels_[(size_t)Panel::screen];

        for (const R& panel : panels_)
        {
                screen.p1.x = std::max(screen.p1.x, panel.p1.x);
                screen.p1.y = std::max(screen.p1.y, panel.p1.y);
        }

        TRACE << "Screen GUI size was set to: "
              << panels::get_w(Panel::screen)
              << ","
              << panels::get_h(Panel::screen)
              << std::endl;
}

static void validate_panels(const P max_gui_dims)
{
        TRACE_FUNC_BEGIN;

        is_valid_ = true;

        const R& screen = panels_[(size_t)Panel::screen];

        for (const R& panel : panels_)
        {
                if ((panel.p1.x >= max_gui_dims.x) ||
                    (panel.p1.y >= max_gui_dims.y) ||
                    (panel.p1.x > screen.p1.x) ||
                    (panel.p1.y > screen.p1.y) ||
                    (panel.p0.x > panel.p1.x) ||
                    (panel.p0.y > panel.p1.y))
                {
                        is_valid_ = false;

                        break;
                }
        }

#ifndef NDEBUG
        if (is_valid_)
        {
                TRACE << "Panels OK" << std::endl;
        }
        else
        {
                TRACE << "Panels NOT OK" << std::endl;
        }
#endif // NDDEBUG

        TRACE_FUNC_END;
}

// -----------------------------------------------------------------------------
// panels
// -----------------------------------------------------------------------------
namespace panels
{

void init(const P max_gui_dims)
{
        TRACE_FUNC_BEGIN;

        TRACE << "Maximum allowed GUI size: "
              << max_gui_dims.x
              << ","
              << max_gui_dims.y
              << std::endl;

        for (auto& panel : panels_)
        {
                panel = R(0, 0, 0, 0);
        }

        set_panel(Panel::player_stats,
                  0,
                  0,
                  6,
                  12);

        set_panel(Panel::equipped,
                  0,
                  7,
                  6,
                  18);

        // (Finalized later, only height matters for now)
        set_panel(Panel::log,
                  0,
                  0,
                  0,
                  1);

        // (Finalized later, only height matters for now)
        set_panel(Panel::properties,
                  0,
                  0,
                  0,
                  0);

        const int map_panel_x = get_w(Panel::player_stats);

        int map_panel_w = max_gui_dims.x - map_panel_x;

        int map_panel_h =
                max_gui_dims.y
                - get_h(Panel::log)
                - get_h(Panel::properties);

        map_panel_w = std::max(map_panel_w, fov_w_int);
        map_panel_h = std::max(map_panel_h, fov_w_int);

        const P gui_cell_px_dims(
                config::gui_cell_px_w(),
                config::gui_cell_px_h());

        const P map_cell_px_dims(
                config::map_cell_px_w(),
                config::map_cell_px_h());

        set_panel(Panel::map,
                  map_panel_x,
                  0,
                  map_panel_x + map_panel_w - 1,
                  map_panel_h - 1);

        set_w(Panel::log, get_w(Panel::map));

        set_x0(Panel::log, get_x0(Panel::map));

        set_y0(Panel::log, get_y1(Panel::map) + 1);

        set_w(Panel::properties, get_w(Panel::map));

        set_x0(Panel::properties, get_x0(Panel::map));

        set_y0(Panel::properties, get_y1(Panel::log) + 1);

        finalize_screen_dims();

        set_panel(Panel::create_char_menu,
                  0,
                  2,
                  23,
                  get_y1(Panel::screen));

        set_panel(Panel::create_char_descr,
                  get_x1(Panel::create_char_menu) + 2,
                  2,
                  get_x1(Panel::screen),
                  get_y1(Panel::screen));

        set_panel(Panel::item_menu,
                  0,
                  1,
                  47,
                  get_y1(Panel::screen) - 1);

        set_panel(Panel::item_descr,
                  get_x1(Panel::item_menu) + 2,
                  1,
                  get_x1(Panel::screen),
                  get_y1(Panel::screen));

        validate_panels(max_gui_dims);

        TRACE_FUNC_END;
}

bool is_valid()
{
        return is_valid_;
}

R get_area(const Panel panel)
{
        return panels_[(size_t)panel];
}

P get_dims(const Panel panel)
{
        return panels_[(size_t)panel].dims();
}

P get_p0(const Panel panel)
{
        return get_area(panel).p0;
}

P get_p1(const Panel panel)
{
        return get_area(panel).p1;
}

int get_x0(const Panel panel)
{
        return get_area(panel).p0.x;
}

int get_y0(const Panel panel)
{
        return get_area(panel).p0.y;
}

int get_x1(const Panel panel)
{
        return get_area(panel).p1.x;
}

int get_y1(const Panel panel)
{
        return get_area(panel).p1.y;
}

int get_w(const Panel panel)
{
        return get_area(panel).w();
}

int get_h(const Panel panel)
{
        return get_area(panel).h();
}

int get_center_x(const Panel panel)
{
        const int x0 = get_x0(panel);
        const int x1 = get_x1(panel);

        return (x1 - x0) / 2;
}

int get_center_y(const Panel panel)
{
        const int y0 = get_y0(panel);
        const int y1 = get_y1(panel);

        return (y1 - y0) / 2;
}

} // panels

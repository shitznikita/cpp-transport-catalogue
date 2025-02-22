#pragma once

#include <cstdint>
#include <iostream>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace svg {

namespace detail {

    template <typename T>
    inline void RenderValue(std::ostream& out, const T& value) {
        out << value;
    }

    void HtmlEncodeString(std::ostream& out, std::string_view sv);

    template <>
    inline void RenderValue<std::string>(std::ostream& out, const std::string& s) {
        HtmlEncodeString(out, s);
    }

    template <typename AttrType>
    inline void RenderAttr(std::ostream& out, std::string_view name, const AttrType& value) {
        using namespace std::literals;
        out << name << "=\""sv;
        RenderValue(out, value);
        out.put('"');
    }

    template <typename AttrType>
    inline void RenderOptionalAttr(std::ostream& out, std::string_view name,
                                    const std::optional<AttrType>& value) {
        if (value) {
            RenderAttr(out, name, *value);
        }
    }

}  // namespace detail

    struct Point {
        Point() = default;
        Point(double x_, double y_)
                : x(x_)
                , y(y_) {
        }
        double x = 0;
        double y = 0;
    };

    struct Rgb {
        int8_t r;
        int8_t g;
        int8_t b;
    };

    struct Rgba {
        int8_t r;
        int8_t g;
        int8_t b;
        double opacity;
    };

    using Color = std::string;
    inline const Color NoneColor{"none"};


    struct RenderContext {
        RenderContext(std::ostream& out_)
                : out(out_) {
        }

        RenderContext(std::ostream& out_, int indent_step_, int indent_ = 0)
                : out(out_)
                , indent_step(indent_step_)
                , indent(indent_) {
        }

        RenderContext Indented() const {
            return {out, indent_step, indent + indent_step};
        }

        void RenderIndent() const {
            for (int i = 0; i < indent; ++i) {
                out.put(' ');
            }
        }

        std::ostream& out;
        int indent_step = 0;
        int indent = 0;
    };


    class Object {
    public:
        void Render(const RenderContext& context) const;

        virtual ~Object() = default;

    private:
        virtual void RenderObject(const RenderContext& context) const = 0;
    };

    enum class StrokeLineCap {
        BUTT,
        ROUND,
        SQUARE,
    };

    std::ostream& operator<<(std::ostream& out, StrokeLineCap value);

    enum class StrokeLineJoin {
        ARCS,
        BEVEL,
        MITER,
        MITER_CLIP,
        ROUND,
    };

    std::ostream& operator<<(std::ostream& out, StrokeLineJoin value);

    template <typename Owner>
    class PathProps {
    public:
        Owner& SetFillColor(Color color) {
            fill_color_ = std::move(color);
            return AsOwner();
        }
        Owner& SetStrokeColor(Color color) {
            stroke_color_ = std::move(color);
            return AsOwner();
        }
        Owner& SetStrokeWidth(double width) {
            stroke_width_ = width;
            return AsOwner();
        }
        Owner& SetStrokeLineCap(StrokeLineCap line_cap) {
            stroke_line_cap_ = line_cap;
            return AsOwner();
        }
        Owner& SetStrokeLineJoin(StrokeLineJoin line_join) {
            stroke_line_join_ = line_join;
            return AsOwner();
        }

    protected:
        ~PathProps() = default;

        void RenderAttrs(std::ostream& out) const {
            using detail::RenderOptionalAttr;
            using namespace std::literals;
            RenderOptionalAttr(out, "fill"sv, fill_color_);
            RenderOptionalAttr(out, " stroke"sv, stroke_color_);
            RenderOptionalAttr(out, " stroke-width"sv, stroke_width_);
            RenderOptionalAttr(out, " stroke-linecap"sv, stroke_line_cap_);
            RenderOptionalAttr(out, " stroke-linejoin"sv, stroke_line_join_);
        }

    private:
        Owner& AsOwner() {
            // static_cast безопасно преобразует *this к Owner&,
            // если класс Owner - наследник PathProps
            return static_cast<Owner&>(*this);
        }

        std::optional<Color> fill_color_;
        std::optional<Color> stroke_color_;
        std::optional<double> stroke_width_;
        std::optional<StrokeLineCap> stroke_line_cap_;
        std::optional<StrokeLineJoin> stroke_line_join_;
    };


    class Circle : public Object, public PathProps<Circle> {
    public:
        Circle& SetCenter(Point center);
        Circle& SetRadius(double radius);

    private:
        void RenderObject(const RenderContext& context) const override;

        Point center_;
        double radius_ = 1.0;
    };


    class Polyline : public Object, public PathProps<Polyline> {
    public:
        // Добавляет очередную вершину к ломаной линии
        Polyline& AddPoint(Point point);

    private:
        void RenderObject(const RenderContext& context) const override;
        std::vector<Point> points_;
    };


    class Text : public Object, public PathProps<Text> {
    public:
        // Задаёт координаты опорной точки (атрибуты x и y)
        Text& SetPosition(Point pos);

        // Задаёт смещение относительно опорной точки (атрибуты dx, dy)
        Text& SetOffset(Point offset);

        // Задаёт размеры шрифта (атрибут font-size)
        Text& SetFontSize(uint32_t size);

        // Задаёт название шрифта (атрибут font-family)
        Text& SetFontFamily(std::string font_family);

        // Задаёт толщину шрифта (атрибут font-weight)
        Text& SetFontWeight(std::string font_weight);

        // Задаёт текстовое содержимое объекта (отображается внутри тэга text)
        Text& SetData(std::string data);

    private:
        void RenderObject(const RenderContext& context) const override;
        Point position_;
        Point offset_;
        uint32_t font_size_ = 1;
        std::string font_family_;
        std::string font_weight_;
        std::string data_;
    };


    // Интерфейс, представляющий контейнер SVG объектов.
    class ObjectContainer {
    public:
        template <typename ObjectType>
        void Add(ObjectType object) {
            AddPtr(std::make_unique<ObjectType>(std::move(object)));
        }

        // Добавляет в svg-документ объект-наследник svg::Object
        virtual void AddPtr(std::unique_ptr<Object>&& obj) = 0;

    protected:
        // Интерфейс не предполагает полиморфное удаление
        // Поэтому деструктор объявлен защищённым невиртуальным
        ~ObjectContainer() = default;
    };

/*
 * Интерфейс объектов, которые могут быть нарисованы на любом объекте,
 * реализующем интерфейс ObjectContainer
 */
    class Drawable {
    public:
        virtual void Draw(ObjectContainer& container) const = 0;

        // Объекты Drawable могут удаляться полиморфно. Поэтому деструктор объявляем публичным
        // виртуальным
        virtual ~Drawable() = default;
    };

    class Document : public ObjectContainer {
    public:
        // Добавляет в svg-документ объект-наследник svg::Object
        void AddPtr(std::unique_ptr<Object>&& obj) override;

        // Выводит в ostream svg-представление документа
        void Render(std::ostream& out) const;

    private:
        std::vector<std::unique_ptr<Object>> objects_;
    };

}  // namespace svg
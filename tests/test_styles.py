import pytest
import viren2d
import pickle


def color_configurations():
    colors = list()
    colors.append(viren2d.Color())
    colors.append(viren2d.rgba(0.3, 0.1, 0.2))
    colors.append(viren2d.Color('blue!40'))
    return colors


def line_style_configurations():
    # Returns various valid/invalid line style configurations
    styles = list()
    style = viren2d.LineStyle()
    styles.append(style.copy())
    for lw in [-2, 0, 0.1, 1, 6]:
        style.line_width = lw
        for color in color_configurations():
            style.color = color
            for dp in [[], [10], [10, 20], [1, 2, 3]]:
                style.dash_pattern = dp

                styles.append(style.copy())
                # Ensure that the list contains an exact copy
                assert styles[-1] == style
                if len(styles) < 2:
                    continue
                # Ensure that we're not accidentally using references
                assert styles[-1] != styles[-2]
    return styles


def test_line_style():
#FIXME init from empty tuple
#FIXME init from tuple
#FIXME init from kwargs
    # Default initialization should yield a valid style
    style = viren2d.LineStyle()
    assert style.is_valid()
    assert not style.is_dashed()

    style.line_width = 0
    assert not style.is_valid()

    style.line_width = 0.4
    assert style.is_valid()

    style.dash_pattern = [20, 10]
    assert style.is_valid()
    assert style.is_dashed()

    style.dash_pattern = []
    assert style.is_valid()
    assert not style.is_dashed()

    style.color = viren2d.Color()
    assert not style.color.is_valid()
    assert not style.is_valid()

    # Comparison
    style = viren2d.LineStyle(2.0, viren2d.Color("magenta"), [],
                              viren2d.LineCap.Round, viren2d.LineJoin.Bevel)
    
    style2 = viren2d.LineStyle(2.0, "magenta", [],
                              viren2d.LineCap.Round, viren2d.LineJoin.Bevel)
    assert style == style2
    style2.line_cap = viren2d.LineCap.Butt
    assert style != style2

    tpl = (2.0, "magenta", [], viren2d.LineCap.Round, viren2d.LineJoin.Bevel)
    assert style == tpl


def test_arrow_style():
#FIXME init from empty tuple
#FIXME init from tuple
#FIXME init from kwargs
    # Default initialization should yield a valid style
    style = viren2d.ArrowStyle()
    assert style.is_valid()

    # Mess around with the base line style
    style.line_width = 0
    assert not style.is_valid()
    style.line_width = 0.4
    assert style.is_valid()

    style.dash_pattern = [3, 7]
    assert style.is_valid()
    assert style.is_dashed()

    style.color = viren2d.Color()
    assert not style.color.is_valid()
    assert not style.is_valid()
    style.color = viren2d.Color("orchid!50")
    assert style.is_valid()

    style.dash_pattern = []
    assert style.is_valid()
    assert not style.is_dashed()

    # Mess around with arrow-specific style
    style.tip_angle = 0
    assert not style.is_valid()
    style.tip_angle = 15
    assert style.is_valid()

    style.tip_length = 0
    assert not style.is_valid()
    style.tip_length = 3
    assert style.is_valid()

    for line_style in line_style_configurations():
        style = viren2d.ArrowStyle(line_style)
        assert line_style.is_valid() == style.is_valid()
        for tl in [-3, 0, 0.3, 5, 500]:
            style.tip_length = tl
            for ta in [-3, 0, 45, 90, 179, 180, 190]:
                style.tip_angle = ta
                for fill in [True, False]:
                    style.tip_closed = fill
                    for dh in [True, False]:
                        style.double_headed = dh
                        assert style.is_valid() == (line_style.is_valid() and (tl > 0) and (ta > 0) and (ta < 180))


def test_arrow_tip_length():
    # Default initialization should yield a valid style
    style = viren2d.ArrowStyle()
    assert style.is_valid()

    style.tip_length = 0
    assert not style.is_valid()

    style.tip_length = -0.1
    assert not style.is_valid()

    style.tip_length = 0.2
    assert style.is_valid()
    assert style.tip_length_for_shaft(200.0) == pytest.approx(40.0)

    style.tip_length = 0.9
    assert style.tip_length_for_shaft(200.0) == pytest.approx(180.0)
    
    style.tip_length = 0.1
    assert style.tip_length_for_shaft(viren2d.Vec2d(90, 100), viren2d.Vec2d(110, 100)) == pytest.approx(2.0)

    style.tip_length = 90
    assert style.tip_length_for_shaft(200.0) == pytest.approx(90.0)
    style.tip_length = 2
    assert style.tip_length_for_shaft(30.0) == pytest.approx(2.0)


def test_line_offsets():
    style = viren2d.ArrowStyle()
    # Default join should be miter:
    assert style.line_join == viren2d.LineJoin.Miter

    # Check offsets at start/end of a line (an ArrowStyle is derived from LineStyle)
    assert style.line_cap == viren2d.LineCap.Butt
    assert style.cap_offset() == pytest.approx(0.0)

    style.line_cap = viren2d.LineCap.Round
    assert style.cap_offset() == pytest.approx(style.line_width / 2.0)
    
    style.line_cap = viren2d.LineCap.Square
    assert style.cap_offset() == pytest.approx(style.line_width / 2.0)

    # Ensure that the object under test matches the style for which we
    # computed the expected values
    style.line_width = 2
    style.line_join = viren2d.LineJoin.Miter    
    assert style.join_offset(10) == pytest.approx(style.line_width / 2.0)
    assert style.join_offset(12) == pytest.approx(9.5667722335056276)
    assert style.join_offset(45) == pytest.approx(2.6131259297527532)

    style.line_join = viren2d.LineJoin.Bevel
    assert style.join_offset(10) == pytest.approx(style.line_width / 2.0)
    assert style.join_offset(12) == pytest.approx(style.line_width / 2.0)
    assert style.join_offset(90) == pytest.approx(style.line_width / 2.0)

    style.line_join = viren2d.LineJoin.Round
    assert style.join_offset(10) == pytest.approx(style.line_width / 2.0)
    assert style.join_offset(12) == pytest.approx(style.line_width / 2.0)
    assert style.join_offset(90) == pytest.approx(style.line_width / 2.0)


def test_line_operators():
    # Compare 2 LineStyle objects
    line_style1 = viren2d.LineStyle()
    line_style2 = viren2d.LineStyle()
    assert line_style1 == line_style2

    line_style2.dash_pattern = [10, 20]
    assert line_style1 != line_style2

    # Compare 2 ArrowStyle objects
    arrow_style1 = viren2d.ArrowStyle()
    arrow_style2 = viren2d.ArrowStyle()
    assert arrow_style1 == arrow_style2

    arrow_style1.tip_length = 30
    assert arrow_style1 != arrow_style2
    arrow_style1.tip_length = arrow_style2.tip_length
    assert arrow_style1 == arrow_style2

    arrow_style2.tip_angle = 23;
    assert arrow_style1 != arrow_style2
    arrow_style2.tip_angle = arrow_style1.tip_angle;
    assert arrow_style1 == arrow_style2

    # ArrowStyle should differ if we change its
    # base LineStyle properties:
    arrow_style2.dash_pattern = [5, 7]
    assert arrow_style1 != arrow_style2
    arrow_style2.dash_pattern = []
    assert arrow_style1 == arrow_style2


def test_pickling():
    # Serialize line style
    for ls in line_style_configurations():
        ls = viren2d.LineStyle(
            2.0, "orchid!30", [], viren2d.LineCap.Round,
            viren2d.LineJoin.Bevel)
        data = pickle.dumps(ls)
        restored = pickle.loads(data)
        assert ls == restored

    # Serialize arrow style
    arr = viren2d.ArrowStyle(
        **ls.as_dict(), tip_length=42, tip_angle=20, tip_closed=True)
    data = pickle.dumps(arr)
    restored = pickle.loads(data)
    assert arr == restored


#include "MapTemplates.h"

#include "Init.h"

#include <vector>
#include <assert.h>

using namespace std;

namespace MapTemplHandling
{

namespace
{

MapTempl templates_[int(MapTemplId::END)];

struct Translation
{
    Translation(char c, const MapTemplCell& mapTemplCell) :
        CH   (c),
        cell (mapTemplCell) {}

    const char          CH;
    const MapTemplCell  cell;
};

MapTemplCell chToCell(const char CH, const vector<Translation>& translations)
{
    for (const Translation& translation : translations)
    {
        if (translation.CH == CH)
        {
            return translation.cell;
        }
    }
    TRACE << "Failed to translate char: " <<  CH << endl;
    assert(false);
    return MapTemplCell();
}

void mkTempl(const string& str, const MapTemplId id,
             const vector<Translation>& translations)
{
    MapTempl& templ = templates_[int(id)];

    vector<MapTemplCell> inner;

    for (const auto ch : str)
    {
        switch (ch)
        {
        case ';':
            //Delimiting character (";") found, inner vector is pushed to outer
            templ.addRow(inner);
            inner.clear();
            break;

        case '#': inner.push_back({FeatureId::wall});           break;
        case '.': inner.push_back({FeatureId::floor});          break;
        case ' ': inner.push_back({});                          break;
        default:  inner.push_back(chToCell(ch, translations));  break;
        }
    }
}

void initTempls()
{
    //Blank level with correct dimensions to copy/paste when making new templates
//  "################################################################################;"
//  "#..............................................................................#;"
//  "#..............................................................................#;"
//  "#..............................................................................#;"
//  "#..............................................................................#;"
//  "#..............................................................................#;"
//  "#..............................................................................#;"
//  "#..............................................................................#;"
//  "#..............................................................................#;"
//  "#..............................................................................#;"
//  "#..............................................................................#;"
//  "#..............................................................................#;"
//  "#..............................................................................#;"
//  "#..............................................................................#;"
//  "#..............................................................................#;"
//  "#..............................................................................#;"
//  "#..............................................................................#;"
//  "#..............................................................................#;"
//  "#..............................................................................#;"
//  "#..............................................................................#;"
//  "#..............................................................................#;"
//  "################################################################################;";

    //Filled version
//  "################################################################################;"
//  "################################################################################;"
//  "################################################################################;"
//  "################################################################################;"
//  "################################################################################;"
//  "################################################################################;"
//  "################################################################################;"
//  "################################################################################;"
//  "################################################################################;"
//  "################################################################################;"
//  "################################################################################;"
//  "################################################################################;"
//  "################################################################################;"
//  "################################################################################;"
//  "################################################################################;"
//  "################################################################################;"
//  "################################################################################;"
//  "################################################################################;"
//  "################################################################################;"
//  "################################################################################;"
//  "################################################################################;"
//  "################################################################################;";

    //----------------------------------------------------------------- CHURCH
    string str =
        "             ,,,,,,,,,,,     ;"
        "          ,,,,,######,,,,    ;"
        " ,,,,,,,,,,,,,,#v..v#,,,,,   ;"
        ",,,,,,,,,,,,,###....###,,,,  ;"
        ",,#####,#,,#,#.#....#.#,,,,, ;"
        ",,#v.v########.#....#.######,;"
        "..#...#v.................v##,;"
        ".,#.#.#..[.[.[.[...[.[....>#,;"
        "..+...+*****************-..#,;"
        ".,#.#.#..[.[.[.[...[.[.....#,;"
        ".,#...#v.................v##,;"
        ",,#v.v########.#....#.######,;"
        ",,#####,#,,#,#.#....#.#,,,,, ;"
        ",,,,,,,,,,,,,###....###,,,,  ;"
        " ,,,,,,,,,,,,,,#v..v#,,,,,   ;"
        "         ,,,,,,######,,,,    ;"
        "            ,,,,,,,,,,,,     ;";

    mkTempl(str, MapTemplId::church, vector<Translation>
    {
        {',', {FeatureId::grass}},
        {'v', {FeatureId::brazier}},
        {'[', {FeatureId::churchBench}},
        {'-', {FeatureId::altar}},
        {'*', {FeatureId::carpet}},
        {'>', {FeatureId::stairs}},
        {'+', {FeatureId::floor,  ActorId::END, ItemId::END, 1}} //Doors
    });

    //----------------------------------------------------------------- EGYPT
    str =
        "################################################################################;"
        "###...################################........................##################;"
        "###.1.###############################..######################..#################;"
        "###...##############################..#########################.################;"
        "####.##############################..####v....################|....|############;"
        "####.#############################..####..###v..##############......############;"
        "####.##########################.....####..######.v############|....|############;"
        "#####.####.#.#.#.#.###########..######v..#######..###############.##############;"
        "######.##|C........|#########.#######..##########..############..###############;"
        "#######.#...........##.....##.#####...############v.##########..################;"
        "########....M...C....#.#.#.#..####..###....########..########..#################;"
        "#########..P.....C.#..........####.####.@...........v#######..##################;"
        "########....M...C....#.#.#.#..##|..|###...#################.v###################;"
        "#######.#...........##.....##.##....######################...###################;"
        "######.##|C........|#########.##|..|########......#######.v##.##################;"
        "#####.####.#.#.#.#.##########.####.########..###v..#####..####.#################;"
        "####.########################.####...#####..#####v..###..######.################;"
        "####.########################..#####..###..#######v.....########.###############;"
        "###...########################...####.....#############.#########.###|...|######;"
        "###.2.##########################...##################v..##########........######;"
        "###...############################v....................##############|...|######;"
        "################################################################################;";

    mkTempl(str, MapTemplId::egypt, vector<Translation>
    {
        {'@', {FeatureId::floor, ActorId::END, ItemId::END, 1}},  //Start
        {'v', {FeatureId::brazier}},
        {'|', {FeatureId::pillar}},
        {'S', {FeatureId::statue}},
        {'P', {FeatureId::floor, ActorId::khephren}},
        {'M', {FeatureId::floor, ActorId::mummy}},
        {'C', {FeatureId::floor, ActorId::crocHeadMummy}},
        {'1', {FeatureId::floor, ActorId::END, ItemId::END, 2}},  //Stair candidate #1
        {'2', {FeatureId::floor, ActorId::END, ItemId::END, 3}}   //Stair candidate #2
    });

    //----------------------------------------------------------------- LENG
    str =
        "%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx;"
        "%%%%%%%%%%-%%--%--%%--%%%--%%--%%-%-%%%--%-%%-x,,,,,x,,,,,x,,,,,x,,,,,,,,,,,,,,x;"
        "%@----%--------%---%---%--------%----%-----%--x,,,,,x,,,,,x,,,,,x,,,,,,,,,,,,,,x;"
        "%%--------------------------------------------xxx,xxxxx,xxxxx,xxx,,,,,,,,,,,,,,x;"
        "%-------------%--------------------------%----x,,,,,,,,,,,,,,,,,x,,,,,,,,,,,,,,x;"
        "%%------------%-------------------------------x,,,xxx,xxxxx,xxx,,,,,,,,,,,,,,,,x;"
        "%%---------------------%%%--------------%-----x,,,x,,,,,x,,,,,xxxxxxxxxxxxxxxxxx;"
        "%%%----%%---------------S%--------------------x,,,x,,,,,x,,,,,xxxxxxxxxxxxxxxxxx;"
        "%%%------------------%%-S%S------%------------x,,,xxxxxxxxxxxxxxxx,,,,,,,,,,,,xx;"
        "%%%%-----------------%--%%-%----%%------------x,,,,,,x,,,x,,,,,,xx,,,,,$,,,,,,xx;"
        "%%%--------------------%%S--------------------xx,xxx,x,,,x,,,,,,xxx,$,,,,,$,,,xx;"
        "%%------------------%-%%S---------------------+,,,,,,x,,,xxxxxx,,,4,,,,,,,,,E,xx;"
        "%%%-------------------S%%S-%------------------xx,xxx,x,,,x,,,,,,xxx,$,,,,,$,,,xx;"
        "%%-------------------%-S%%-%----%-------------x,,,x,,xxx,x,xxxxxxx,,,,,$,,,,,,xx;"
        "%%---------------------S%%S-------------------x,,,x,,,,,,x,,,,,,xx,,,,,,,,,,,,xx;"
        "%%%------------------%-%%---------------%-----x,xxxxxxxxxxxxxxx,xxxxxxxxxxxxx5xx;"
        "%%-----%---------------------------%----------x,x,,,,,,,,,,,,,,,xxxxxxxxxxxxx,xx;"
        "%%%---------%---------------------------------x,x,xxx,xxx,xxx,x,xxxx,,,,xxxxx,,x;"
        "%%--%-----------------------------------------x,x,xxx,xxx,xxx,x,xx,,,xx,,,xxxx,x;"
        "%%%--------------%----------------%---%-----%-x,x,xxx,xxx,xxx,x,x,,xxxxxx,,,,x,x;"
        "%%%%%%--%---%--%%%-%--%%%%%%%-%-%%%--%%--%-%%%x,,,,,,,,,,,,,,,x,,,xxxxxxxxxx,,,x;"
        "%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx;";

    mkTempl(str, MapTemplId::leng, vector<Translation>
    {
        {'@', {FeatureId::floor,  ActorId::END,       ItemId::END, 1}}, //Start
        {'%', {FeatureId::wall,   ActorId::END,       ItemId::END, 2}},
        {'x', {FeatureId::wall,   ActorId::END,       ItemId::END, 3}},
        {',', {FeatureId::floor,  ActorId::END,       ItemId::END, 3}},
        {'E', {FeatureId::floor,  ActorId::lengElder, ItemId::END, 3}},
        {'4', {FeatureId::floor,  ActorId::END,       ItemId::END, 4}},
        {'5', {FeatureId::wall,   ActorId::END,       ItemId::END, 5}},
        {'$', {FeatureId::altar}},
        {'-', {FeatureId::grass}},
        {'+', {FeatureId::floor,  ActorId::END,       ItemId::END, 6}}, //Door
        {'S', {FeatureId::grass,  ActorId::lengSpider}}
    });

    //----------------------------------------------------------------- RATS IN THE WALLS
    str =
        "################################################################################;"
        "##@#################,##,##xxxxxxxxx###xxxxxxxxxxx######rr#,##########,#,########;"
        "##.##############,,,,,,,,,x,,,,,,,xrrrxrr,rrr,rrxr,rrrr,,,,,,##,,#,,,,,,,#######;"
        "##...&##########,,,,xxxxxxx,,,,,,,xr,rxr,,,,,,,rxrrr,,,,,:,,,,,,,,,,,:,,,,######;"
        "###..:#########,,:,,x,,,,,,,,,,,,,,,,rrrrxx,xxrrxr,r,,,,,,,,,,,,,,,,,,,,,,,,,###;"
        "###:...#######,,,,,,xx,xxxx,,,,,,,xrrrxrrx,,,xrrxrrr,,,,,,,,:,,,,,,:,,,,,,######;"
        "##&..:..#####,,,,,,,,,,,,,xxxx,xxxx,r,xxxx,,,x,xx,,,,,,,,,,,,,,,,,,,,,,,,,######;"
        "####.&.:####,,,,,,,,,,,,,,,:,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,:,,,,,####;"
        "####..##&:3.,,,,,,,,,:,,,,,,,,,,:,,,,,,,,,,,1,,,:,,,xxx,xx,xxx,,,,,,,,,,,,,#####;"
        "#####..&:.#,,,,,,,,,,,,,,,x,x,x,x,x,x,,,,1,,,,,1,,,rx,,,,,,,,x,,,,:,,,,,,,######;"
        "###########,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,x,rrrrr,,x,,,,,,,,,,,#######;"
        "###########,,,,,,,,1,,,,,,,,,,,,,,,,:,,,1,,,,,,,1,,rr,,,,,r,,,,,,,#,,,,,########;"
        "###########,,,,,1,,,,,1,,,,,,,,,,,,,,,,,,,,,,,,,,,,,x,r,rr,,,x,,####,,,,,#######;"
        "###########,,,,,,,,,,,,,,,x,x,x,x,x,x,,,,1,,,,,1,,,rx,,rrr,,,x,,,,##############;"
        "############,,,1,,,,,,,1,,,,,,,,,,,,,,,,,,,,1,,,,,x,x,r,,r,,,x,,,###############;"
        "############,,,,,,,,,,,,,,,,,,,,:,,,,,,,,,,,,,,,,,rr,,,rr,,,,x,:##xxxxxx########;"
        "#############,,,1,,,,,1,,,xxx,xxxxx,xxx,,,:,,,,r,,,rx,rr,,,,,x,,,,x....x########;"
        "##############,,,,,1,,,,,,x,,,,,xrrrrrx,,,,,,,,,,,r,x,r,,,,,rx,,,,...>.x########;"
        "#################,,,,,,:,,x,,,,,xrr,,rxrr,rr,rr,r,,,xxxxxxxxxx,:,,x....x########;"
        "################:,,,,,,,,,x,,,,,xrrrrrxrrrr##rrr,r,,rr,rr,rr,r,##:xxxxxx########;"
        "###################:,,####xxxxxxxxxxxxx##r#####rr###r###r#rrr###################;"
        "################################################################################;";

    mkTempl(str, MapTemplId::ratsInTheWalls, vector<Translation>
    {
        {'@', {FeatureId::floor,  ActorId::END,     ItemId::END, 1}}, //Start
        {'x', {FeatureId::wall,   ActorId::END,     ItemId::END, 2}}, //Constructed walls
        {'&', {FeatureId::bones,  ActorId::END,     ItemId::END}},
        {'3', {FeatureId::floor,  ActorId::END,     ItemId::END, 3}}, //Discovery event
        {',', {FeatureId::floor,  ActorId::END,     ItemId::END, 4}}, //Random bones
        {'r', {FeatureId::floor,  ActorId::rat,     ItemId::END, 4}}, //Random bones + rat
        {'>', {FeatureId::stairs}},
        {'1', {FeatureId::monolith}},
        {':', {FeatureId::stalagmite}}
    });

    //----------------------------------------------------------------- BOSS LEVEL
    str =
        "################################################################################;"
        "############################...................................................#;"
        "############################...................................................#;"
        "############################...#.....#...#.....#...#.....#...#.....#...........#;"
        "############################...###.###...###.###...###.###...###.###...........#;"
        "##############v..v#########......###.......###.......###.......###.............#;"
        "##############....#########....###.###...###.###...###.###...###.###...........#;"
        "############.#....#.#######....#.....#...#.....#...#.....#...#.....#...........#;"
        "#v.v########.#....#.#######.................................................####;"
        "#...#v.................v#v#.......|....|....|....|....|....|....|...|...|...####;"
        "#.#.#.............................................................v...v...v..###;"
        "#@..........................................................................M###;"
        "#.#.#.............................................................v...v...v..###;"
        "#...#v.................v#v#.......|....|....|....|....|....|....|...|...|...####;"
        "#v.v########.#....#.#######.................................................####;"
        "############.#....#.#######....#.....#...#.....#...#.....#...#.....#...........#;"
        "##############....#########....###.###...###.###...###.###...###.###...........#;"
        "##############v..v#########......###.......###.......###.......###.............#;"
        "############################...###.###...###.###...###.###...###.###...........#;"
        "############################...#.....#...#.....#...##...##...#.....#...........#;"
        "############################...................................................#;"
        "################################################################################;";

    mkTempl(str, MapTemplId::bossLevel, vector<Translation>
    {
        {'@', {FeatureId::floor,    ActorId::END,         ItemId::END, 1}}, //Start
        {'M', {FeatureId::floor,    ActorId::theDarkOne,  ItemId::END}},    //Boss
        {'|', {FeatureId::pillar,   ActorId::END,         ItemId::END}},
        {'v', {FeatureId::brazier,  ActorId::END,         ItemId::END}},
        {'>', {FeatureId::stairs,   ActorId::END,         ItemId::END}}
    });
}

} //namespace

void init()
{
    initTempls();
}

const MapTempl& getTempl(const MapTemplId id)
{
    assert(id != MapTemplId::END);
    return templates_[int(id)];
}

} //MapTemplHandling

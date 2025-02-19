/*
 * Copyright (c) 2024
 *	Side Effects Software Inc.  All rights reserved.
 *
 * Redistribution and use of Houdini Development Kit samples in source and
 * binary forms, with or without modification, are permitted provided that the
 * following conditions are met:
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. The name of Side Effects Software may not be used to endorse or
 *    promote products derived from this software without specific prior
 *    written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY SIDE EFFECTS SOFTWARE `AS IS' AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN
 * NO EVENT SHALL SIDE EFFECTS SOFTWARE BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
 * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *----------------------------------------------------------------------------
 * The VmdFile SOP
 */

#include "SOP_VmdFile.h"
#include "SOP_common.h"

// This is an automatically generated header file based on theDsFile, below,
// to provide SOP_VmdFileParms, an easy way to access parameter values from
// SOP_VmdFileVerb::cook with the correct type.
#include "SOP_VmdFile.proto.h"

#include <GU/GU_Detail.h>
#include <OP/OP_Operator.h>
#include <OP/OP_OperatorTable.h>
#include <PRM/PRM_Include.h>
#include <PRM/PRM_TemplateBuilder.h>
#include <UT/UT_DSOVersion.h>
#include <UT/UT_Interrupt.h>
#include <UT/UT_StringHolder.h>
#include <SOP/SOP_Node.h>
#include <GU/GU_Detail.h>
#include <GA/GA_Attribute.h>
#include <GA/GA_AIFSharedStringTuple.h>
#include <UT/UT_StringArray.h>
#include <SYS/SYS_Math.h>
#include <SYS/SYS_Types.h>
#include <limits.h>
#include <iostream>
#include <typeinfo>
#include <algorithm> 
#include <stdint.h>
#include <windows.h>
#include <fmt/core.h>

using namespace UT::Literal;
using namespace HDK_Sample;

//
// Help is stored in a "wiki" style text file.  This text file should be copied
// to $HOUDINI_PATH/help/nodes/sop/star.txt
//
// See the sample_install.sh file for an example.
//

/// This is the internal name of the SOP type.
/// It isn't allowed to be the same as any other SOP's type name.
const UT_StringHolder SOP_VmdFile::theSOPTypeName("mmd_vmd_file"_sh);

/// newSopOperator is the hook that Houdini grabs from this dll
/// and invokes to register the SOP.  In this case, we add ourselves
/// to the specified operator table.
void
newSopOperator(OP_OperatorTable *table)
{
    table->addOperator(new OP_Operator(
        SOP_VmdFile::theSOPTypeName,   // Internal name
        "VmdFile",                     // UI name
        SOP_VmdFile::myConstructor,    // How to build the SOP
        SOP_VmdFile::buildTemplates(), // My parameters
        0,                          // Min # of sources
        0,                          // Max # of sources
        nullptr,                    // Custom local variables (none)
        OP_FLAG_GENERATOR));        // Flag it as generator
}

/// This is a multi-line raw string specifying the parameter interface
/// for this SOP.
static const char *theDsFile = R"THEDSFILE(
{
    name        parameters
    parm {
        name    "vmd_path"
        label   "VMD Path"
        type    file
        default { "D:/IA_Conqueror_light_version.vmd" }
        parmtag { "filechooser_pattern" "*.vmd" }
        parmtag { "filechooser_mode" "read" }
    }
    parm {
        name    "scale"
        label   "Scale"
        type    float
        default { "0.08" }
    }
}
)THEDSFILE";

PRM_Template*
SOP_VmdFile::buildTemplates()
{
    static PRM_TemplateBuilder templ("SOP_VmdFile.C"_sh, theDsFile);
    return templ.templates();
}

class SOP_VmdFileVerb : public SOP_NodeVerb
{
public:
    SOP_VmdFileVerb() {}
    virtual ~SOP_VmdFileVerb() {}

    virtual SOP_NodeParms *allocParms() const { return new SOP_VmdFileParms(); }
    virtual UT_StringHolder name() const { return SOP_VmdFile::theSOPTypeName; }

    virtual CookMode cookMode(const SOP_NodeParms *parms) const { return COOK_GENERIC; }

    virtual void cook(const CookParms &cookparms) const;
    
    /// This static data member automatically registers
    /// this verb class at library load time.
    static const SOP_NodeVerb::Register<SOP_VmdFileVerb> theVerb;
};

// The static member variable definition has to be outside the class definition.
// The declaration is inside the class.
const SOP_NodeVerb::Register<SOP_VmdFileVerb> SOP_VmdFileVerb::theVerb;

const SOP_NodeVerb *
SOP_VmdFile::cookVerb() const 
{ 
    return SOP_VmdFileVerb::theVerb.get();
}


// Shift-JIS to UTF-8 conversion function
static std::string ShiftJISToUTF8(const std::string shiftJISStr) {
    // Step 1: Convert Shift-JIS to UTF-16
    int wideCharLen = MultiByteToWideChar(932, 0, shiftJISStr.c_str(), -1, NULL, 0);
    if (wideCharLen == 0) {
        return ""; // Conversion failed
    }
    std::wstring wideStr(wideCharLen, 0);
    MultiByteToWideChar(932, 0, shiftJISStr.c_str(), -1, &wideStr[0], wideCharLen);

    // Step 2: Convert UTF-16 to UTF-8
    int utf8Len = WideCharToMultiByte(CP_UTF8, 0, wideStr.c_str(), -1, NULL, 0, NULL, NULL);
    if (utf8Len == 0) {
        return ""; // Conversion failed
    }
    std::string utf8Str(utf8Len, 0);
    WideCharToMultiByte(CP_UTF8, 0, wideStr.c_str(), -1, &utf8Str[0], utf8Len, NULL, NULL);
    if (utf8Str.size() > 0) {
        utf8Str.pop_back();
    }

    return utf8Str;
}

struct BoneFrame {
    glm::vec3 trans;
    glm::quat rot;
};

struct Bezier {
    glm::vec2 p1;
    glm::vec2 p2;
    Bezier() {}

    Bezier(glm::vec4 c) {
        p1 = {c[0], c[1]};
        p2 = {c[2], c[3]};
    }

    glm::vec2 eval(float t) const {
        glm::vec2 res_p1 = p1 * (3.0f * t * (1.0f - t) * (1.0f - t));
        glm::vec2 res_p2 = p2 * (3.0f * t * t * (1.0f - t));
        float i3 = t * t * t;
        return {
            res_p1.x + res_p2.x + i3,
            res_p1.y + res_p2.y + i3
        };
    }

    float find_t_when_x(float x) const {
        const float epsilon = 0.00001f;
        float start = 0.0f;
        float end = 1.0f;
        float t = 0.5f;
        float cur_x = eval(t).x;
        int bezier_max_count = 100;

        while (glm::abs(x - cur_x) > epsilon && bezier_max_count > 0) {
            if (x < cur_x) {
                end = t;
            } else {
                start = t;
            }
            t = (start + end) * 0.5f;
            cur_x = eval(t).x;
            --bezier_max_count;
        }
        return t;
    }

    float find_y_when_x(float x) const {
        float t = find_t_when_x(x);
        return eval(t).y;
    }
};


struct BoneKeyframe {
    uint32_t frame;
    glm::vec3 trans;
    glm::quat rot;
    Bezier txc;
    Bezier tyc;
    Bezier tzc;
    Bezier rc;
    BoneFrame to_bone_frame() {
        BoneFrame bf;
        bf.trans = trans;
        bf.rot = rot;
        return bf;
    }
    bool is_zero() {
        return trans[0] == 0 && trans[1] == 0 && trans[2] == 0
            && rot[0] == 0 && rot[1] == 0 && rot[2] == 0 && rot[3] == 1;
    }
    BoneFrame lerp(BoneKeyframe &next_keyframe, float t) {
        BoneFrame bf;
        auto &n = next_keyframe;
        float t_tx = n.txc.find_y_when_x(t);
        float t_ty = n.tyc.find_y_when_x(t);
        float t_tz = n.tzc.find_y_when_x(t);
        float t_r  = n.rc .find_y_when_x(t);
        bf.trans = {
            glm::lerp(trans.x, n.trans.x, t_tx),
            glm::lerp(trans.y, n.trans.y, t_ty),
            glm::lerp(trans.z, n.trans.z, t_tz),
        };
        bf.rot = glm::slerp(rot, n.rot, t_r);
        return bf;
    }
};

static glm::vec4 read_bezier_control_point_pair4(BinaryReader &br) {
    float x = float(br.read_LE<uint32_t>() & 0xFF) / 127.0;
    float y = float(br.read_LE<uint32_t>() & 0xFF) / 127.0;
    float z = float(br.read_LE<uint32_t>() & 0xFF) / 127.0;
    float w = float(br.read_LE<uint32_t>() & 0xFF) / 127.0;
    return {x, y, z, w};
}

static void read_anim(BinaryReader &br, GU_Detail *detail, float scale) {
    auto header = br.read_string(30);
    std::string name = br.read_string(header[24] == '2'? 20 : 10);
    name = ShiftJISToUTF8(name);

    setUserData(detail, "vmd_name", name);

    auto count = br.read_LE<uint32_t>();
    std::map<std::string, std::vector<BoneKeyframe>> bone_keyframes;
    for (auto i = 0; i < count; i++) {
        BoneKeyframe bk;
        auto name = br.read_string(15);
        name = ShiftJISToUTF8(name);
        bk.frame = br.read_LE<uint32_t>();
        bk.trans = br.read_float3() * glm::vec3(1, 1, -1) * scale;
        bk.rot = br.read_quat();
        bk.rot.x *= -1;
        bk.rot.y *= -1;
        bk.txc = Bezier(read_bezier_control_point_pair4(br));
        bk.tyc = Bezier(read_bezier_control_point_pair4(br));
        bk.tzc = Bezier(read_bezier_control_point_pair4(br));
        bk.rc  = Bezier(read_bezier_control_point_pair4(br));
        bone_keyframes[name].push_back(bk);
    }
    {
        std::vector<std::string> need_remove_names;
        for (auto &[name, kfs]: bone_keyframes) {
            bool need_remove = true;
            for (auto &kf: kfs) {
                if (kf.is_zero() == false) {
                    need_remove = false;
                    break;
                }
            }
            if (need_remove) {
                need_remove_names.push_back(name);
            }
            else {
                std::sort(kfs.begin(), kfs.end(), [](const auto& a, const auto& b) {
                    return a.frame < b.frame;
                });
            }
        }
        for (auto &name: need_remove_names) {
            bone_keyframes.erase(name);
        }
    }
    int total_count = 0;
    for (auto &[name, kfs]: bone_keyframes) {
        total_count += kfs.size();
    }

    GA_Offset start_ptoff{};
    start_ptoff = detail->appendPointBlock(total_count);
    auto frame_attr = GA_RWHandleI(detail->addIntTuple(GA_ATTRIB_POINT, "frame", 1));
    auto rot_attr = GA_RWHandleQ(detail->addFloatTuple(GA_ATTRIB_POINT, "rot", 4));
    auto txc_attr = GA_RWHandleV4(detail->addFloatTuple(GA_ATTRIB_POINT, "txc", 4));
    auto tyc_attr = GA_RWHandleV4(detail->addFloatTuple(GA_ATTRIB_POINT, "tyc", 4));
    auto tzc_attr = GA_RWHandleV4(detail->addFloatTuple(GA_ATTRIB_POINT, "tzc", 4));
    auto rc_attr = GA_RWHandleV4(detail->addFloatTuple(GA_ATTRIB_POINT, "rc", 4));
    auto name_attr = GA_RWHandleS(detail->addStringTuple(GA_ATTRIB_POINT , "name", 1));
    int index = 0;
    for (auto &[name, kfs]: bone_keyframes) {
        UT_StringHolder name_sh = name;
        for (auto &kf: kfs) {
            detail->setPos3(start_ptoff + index, UT_Vector3(kf.trans[0], kf.trans[1], kf.trans[2]));
            frame_attr.set(start_ptoff + index, kf.frame);
            rot_attr.set(start_ptoff + index, UT_QuaternionF(kf.rot[0], kf.rot[1], kf.rot[2], kf.rot[3]));
            txc_attr.set(start_ptoff + index, UT_Vector4(kf.txc.p1[0], kf.txc.p1[1], kf.txc.p2[0], kf.txc.p2[1]));
            tyc_attr.set(start_ptoff + index, UT_Vector4(kf.tyc.p1[0], kf.tyc.p1[1], kf.tyc.p2[0], kf.tyc.p2[1]));
            tzc_attr.set(start_ptoff + index, UT_Vector4(kf.tzc.p1[0], kf.tzc.p1[1], kf.tzc.p2[0], kf.tzc.p2[1]));
            rc_attr .set(start_ptoff + index, UT_Vector4(kf.rc .p1[0], kf.rc .p1[1], kf.rc .p2[0], kf.rc .p2[1]));
            name_attr.set(start_ptoff + index, name_sh);
            index += 1;
        }
    }
}


/// This is the function that does the actual work.
void
SOP_VmdFileVerb::cook(const SOP_NodeVerb::CookParms &cookparms) const
{
    auto &&sopparms = cookparms.parms<SOP_VmdFileParms>();

    float scale = sopparms.getScale();
    GU_Detail *detail = cookparms.gdh().gdpNC();
    detail->clearAndDestroy();
    auto file_path = sopparms.getVmd_path().toStdString();
    if (file_path.empty()) {
        return;
    }
    setUserData(detail, "file_path", file_path);
    auto bin = file_get_binary(file_path);
    auto br = BinaryReader(bin);
    read_anim(br, detail, scale);
    detail->bumpDataIdsForAddOrRemove(true, true, true);
}

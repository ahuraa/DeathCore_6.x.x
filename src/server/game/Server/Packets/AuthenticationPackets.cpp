/*
 * Copyright (C) 2013-2015 DeathCore <http://www.noffearrdeathproject.net/>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "AuthenticationPackets.h"
#include "HmacHash.h"

WorldPacket const* WorldPackets::Auth::AuthChallenge::Write()
{
    _worldPacket << uint32(Challenge);
    _worldPacket.append(DosChallenge, 8);
    _worldPacket << uint8(DosZeroBits);
    return &_worldPacket;
}

void WorldPackets::Auth::AuthSession::Read()
{
    uint32 addonDataSize;

    _worldPacket >> LoginServerID;
    _worldPacket >> Build;
    _worldPacket >> RegionID;
    _worldPacket >> BattlegroupID;
    _worldPacket >> RealmID;
    _worldPacket >> LoginServerType;
    _worldPacket >> BuildType;
    _worldPacket >> LocalChallenge;
    _worldPacket >> DosResponse;
    _worldPacket.read(Digest, SHA_DIGEST_LENGTH);
    Account = _worldPacket.ReadString(_worldPacket.ReadBits(11));
    UseIPv6 = _worldPacket.ReadBit();           // UseIPv6
    _worldPacket >> addonDataSize;
    if (addonDataSize)
    {
        AddonInfo.resize(addonDataSize);
        _worldPacket.read(AddonInfo.contents(), addonDataSize);
    }
}

WorldPackets::Auth::AuthResponse::AuthResponse()
    : ServerPacket(SMSG_AUTH_RESPONSE, 132)
{
    WaitInfo.HasValue = false;
    SuccessInfo.HasValue = false;
}

WorldPacket const* WorldPackets::Auth::AuthResponse::Write()
{
    _worldPacket << uint8(Result);
    _worldPacket.WriteBit(SuccessInfo.HasValue);
    _worldPacket.WriteBit(WaitInfo.HasValue);

    if (SuccessInfo.HasValue)
    {
        _worldPacket << uint32(SuccessInfo.Value.VirtualRealmAddress);
        _worldPacket << uint32(SuccessInfo.Value.VirtualRealms.size());
        _worldPacket << uint32(SuccessInfo.Value.TimeRemain);
        _worldPacket << uint32(SuccessInfo.Value.TimeOptions);
        _worldPacket << uint32(SuccessInfo.Value.TimeRested);
        _worldPacket << uint8(SuccessInfo.Value.ActiveExpansionLevel);
        _worldPacket << uint8(SuccessInfo.Value.AccountExpansionLevel);
        _worldPacket << uint32(SuccessInfo.Value.TimeSecondsUntilPCKick);
        _worldPacket << uint32(SuccessInfo.Value.AvailableRaces->size());
        _worldPacket << uint32(SuccessInfo.Value.AvailableClasses->size());
        _worldPacket << uint32(SuccessInfo.Value.Templates.size());
        _worldPacket << uint32(SuccessInfo.Value.CurrencyID);

        for (auto& realm : SuccessInfo.Value.VirtualRealms)
        {
            _worldPacket << uint32(realm.RealmAddress);
            _worldPacket.WriteBit(realm.IsLocal);
            _worldPacket.WriteBit(realm.IsInternalRealm);
            _worldPacket.WriteBits(realm.RealmNameActual.length(), 8);
            _worldPacket.WriteBits(realm.RealmNameNormalized.length(), 8);
            _worldPacket.WriteString(realm.RealmNameActual);
            _worldPacket.WriteString(realm.RealmNameNormalized);
        }

        for (auto& race : *SuccessInfo.Value.AvailableRaces)
        {
            _worldPacket << uint8(race.first); /// the current race
            _worldPacket << uint8(race.second); /// the required Expansion
        }

        for (auto& klass : *SuccessInfo.Value.AvailableClasses)
        {
            _worldPacket << uint8(klass.first); /// the current class
            _worldPacket << uint8(klass.second); /// the required Expansion
        }

        for (auto& templat : SuccessInfo.Value.Templates)
        {
            _worldPacket << uint32(templat.TemplateSetId);
            _worldPacket << uint32(templat.TemplateClasses.size());
            for (auto& templatClass : templat.TemplateClasses)
            {
                _worldPacket << uint8(templatClass.Class);
                _worldPacket << uint8(templatClass.FactionGroup);
            }

            _worldPacket.WriteBits(templat.Name.length(), 7);
            _worldPacket.WriteBits(templat.Description.length(), 10);
            _worldPacket.WriteString(templat.Name);
            _worldPacket.WriteString(templat.Description);
        }

        _worldPacket.WriteBit(SuccessInfo.Value.IsExpansionTrial);
        _worldPacket.WriteBit(SuccessInfo.Value.ForceCharacterTemplate);
        _worldPacket.WriteBit(SuccessInfo.Value.NumPlayersHorde.HasValue);
        _worldPacket.WriteBit(SuccessInfo.Value.NumPlayersAlliance.HasValue);
        _worldPacket.WriteBit(SuccessInfo.Value.IsVeteranTrial);

        if (SuccessInfo.Value.NumPlayersHorde.HasValue)
            _worldPacket << uint16(SuccessInfo.Value.NumPlayersHorde.Value);

        if (SuccessInfo.Value.NumPlayersAlliance.HasValue)
            _worldPacket << uint16(SuccessInfo.Value.NumPlayersAlliance.Value);
    }

    if (WaitInfo.HasValue)
    {
        _worldPacket << uint32(WaitInfo.Value.WaitCount);
        _worldPacket.WriteBit(WaitInfo.Value.HasFCM);
    }

    _worldPacket.FlushBits();
    return &_worldPacket;
}

std::string const WorldPackets::Auth::ConnectTo::Haiku("An island of peace\nCorruption is brought ashore\nPandarens will rise\n\0\0", 71);

uint8 const WorldPackets::Auth::ConnectTo::PiDigits[130] =
{
    0x31, 0x41, 0x59, 0x26, 0x53, 0x58, 0x97, 0x93, 0x23, 0x84,
    0x62, 0x64, 0x33, 0x83, 0x27, 0x95, 0x02, 0x88, 0x41, 0x97,
    0x16, 0x93, 0x99, 0x37, 0x51, 0x05, 0x82, 0x09, 0x74, 0x94,
    0x45, 0x92, 0x30, 0x78, 0x16, 0x40, 0x62, 0x86, 0x20, 0x89,
    0x98, 0x62, 0x80, 0x34, 0x82, 0x53, 0x42, 0x11, 0x70, 0x67,
    0x98, 0x21, 0x48, 0x08, 0x65, 0x13, 0x28, 0x23, 0x06, 0x64,
    0x70, 0x93, 0x84, 0x46, 0x09, 0x55, 0x05, 0x82, 0x23, 0x17,
    0x25, 0x35, 0x94, 0x08, 0x12, 0x84, 0x81, 0x11, 0x74, 0x50,
    0x28, 0x41, 0x02, 0x70, 0x19, 0x38, 0x52, 0x11, 0x05, 0x55,
    0x96, 0x44, 0x62, 0x29, 0x48, 0x95, 0x49, 0x30, 0x38, 0x19,
    0x64, 0x42, 0x88, 0x10, 0x97, 0x56, 0x65, 0x93, 0x34, 0x46,
    0x12, 0x84, 0x75, 0x64, 0x82, 0x33, 0x78, 0x67, 0x83, 0x16,
    0x52, 0x71, 0x20, 0x19, 0x09, 0x14, 0x56, 0x48, 0x56, 0x69,
};

/*
RSA key values

uint8 const Modulus[] =
{
    0x5F, 0xD6, 0x80, 0x0B, 0xA7, 0xFF, 0x01, 0x40, 0xC7, 0xBC, 0x8E, 0xF5, 0x6B, 0x27, 0xB0, 0xBF,
    0xF0, 0x1D, 0x1B, 0xFE, 0xDD, 0x0B, 0x1F, 0x3D, 0xB6, 0x6F, 0x1A, 0x48, 0x0D, 0xFB, 0x51, 0x08,
    0x65, 0x58, 0x4F, 0xDB, 0x5C, 0x6E, 0xCF, 0x64, 0xCB, 0xC1, 0x6B, 0x2E, 0xB8, 0x0F, 0x5D, 0x08,
    0x5D, 0x89, 0x06, 0xA9, 0x77, 0x8B, 0x9E, 0xAA, 0x04, 0xB0, 0x83, 0x10, 0xE2, 0x15, 0x4D, 0x08,
    0x77, 0xD4, 0x7A, 0x0E, 0x5A, 0xB0, 0xBB, 0x00, 0x61, 0xD7, 0xA6, 0x75, 0xDF, 0x06, 0x64, 0x88,
    0xBB, 0xB9, 0xCA, 0xB0, 0x18, 0x8B, 0x54, 0x13, 0xE2, 0xCB, 0x33, 0xDF, 0x17, 0xD8, 0xDA, 0xA9,
    0xA5, 0x60, 0xA3, 0x1F, 0x4E, 0x27, 0x05, 0x98, 0x6F, 0xAA, 0xEE, 0x14, 0x3B, 0xF3, 0x97, 0xA8,
    0x12, 0x02, 0x94, 0x0D, 0x84, 0xDC, 0x0E, 0xF1, 0x76, 0x23, 0x95, 0x36, 0x13, 0xF9, 0xA9, 0xC5,
    0x48, 0xDB, 0xDA, 0x86, 0xBE, 0x29, 0x22, 0x54, 0x44, 0x9D, 0x9F, 0x80, 0x7B, 0x07, 0x80, 0x30,
    0xEA, 0xD2, 0x83, 0xCC, 0xCE, 0x37, 0xD1, 0xD1, 0xCF, 0x85, 0xBE, 0x91, 0x25, 0xCE, 0xC0, 0xCC,
    0x55, 0xC8, 0xC0, 0xFB, 0x38, 0xC5, 0x49, 0x03, 0x6A, 0x02, 0xA9, 0x9F, 0x9F, 0x86, 0xFB, 0xC7,
    0xCB, 0xC6, 0xA5, 0x82, 0xA2, 0x30, 0xC2, 0xAC, 0xE6, 0x98, 0xDA, 0x83, 0x64, 0x43, 0x7F, 0x0D,
    0x13, 0x18, 0xEB, 0x90, 0x53, 0x5B, 0x37, 0x6B, 0xE6, 0x0D, 0x80, 0x1E, 0xEF, 0xED, 0xC7, 0xB8,
    0x68, 0x9B, 0x4C, 0x09, 0x7B, 0x60, 0xB2, 0x57, 0xD8, 0x59, 0x8D, 0x7F, 0xEA, 0xCD, 0xEB, 0xC4,
    0x60, 0x9F, 0x45, 0x7A, 0xA9, 0x26, 0x8A, 0x2F, 0x85, 0x0C, 0xF2, 0x19, 0xC6, 0x53, 0x92, 0xF7,
    0xF0, 0xB8, 0x32, 0xCB, 0x5B, 0x66, 0xCE, 0x51, 0x54, 0xB4, 0xC3, 0xD3, 0xD4, 0xDC, 0xB3, 0xEE
};

uint8 const Exponent[] = { 0x01, 0x00, 0x01, 0x00 };

uint8 const D[] =
{
    0x79, 0x16, 0xC0, 0xDB, 0xF2, 0x31, 0xCE, 0xA7, 0xEB, 0xFB, 0x91, 0x1F, 0x1E, 0x72, 0x70, 0x25,
    0x0B, 0xF5, 0x7A, 0xE0, 0x88, 0x0B, 0x79, 0xBD, 0xFF, 0xBA, 0x24, 0x62, 0x5A, 0x08, 0x59, 0x2B,
    0x41, 0x7E, 0x4B, 0xF4, 0x9C, 0x3D, 0x4B, 0x5C, 0xEA, 0x6B, 0x21, 0xB8, 0x6E, 0x1C, 0xD1, 0x30,
    0x3E, 0x7C, 0x9C, 0x74, 0xA9, 0x9F, 0x77, 0x31, 0x28, 0xAE, 0x0C, 0x65, 0x18, 0xFF, 0x32, 0x63,
    0x06, 0xD9, 0x33, 0x03, 0xEA, 0x31, 0x26, 0x06, 0x2E, 0xF9, 0x20, 0x81, 0x07, 0xEB, 0x04, 0x42,
    0x22, 0x31, 0x5C, 0x7D, 0x6E, 0x5B, 0x04, 0xF0, 0xBB, 0x4E, 0xF6, 0xB5, 0x9B, 0x96, 0x56, 0xBD,
    0x4C, 0x0E, 0x79, 0xF4, 0x8F, 0x8F, 0xF1, 0xEA, 0x35, 0x5C, 0x98, 0x23, 0x29, 0xA7, 0x7C, 0xAC,
    0xF5, 0xD3, 0x50, 0x2D, 0xDA, 0xB3, 0x5A, 0x34, 0x33, 0x4D, 0x02, 0x9E, 0x39, 0xAD, 0x52, 0x9B,
    0xCE, 0x78, 0xAD, 0x6F, 0x65, 0xD4, 0x81, 0xA3, 0x64, 0x0D, 0x6B, 0x96, 0x29, 0x1C, 0x6C, 0xE3,
    0x1D, 0xDA, 0x4C, 0x5E, 0xAF, 0xD7, 0x14, 0x2A, 0xC9, 0x07, 0x23, 0x04, 0x2F, 0xC4, 0x73, 0x4B,
    0xDC, 0xCC, 0xEE, 0x94, 0xE8, 0xFE, 0xF3, 0x09, 0x88, 0x8E, 0xF6, 0xF4, 0x31, 0x5D, 0xC1, 0xA4,
    0x3B, 0x54, 0x47, 0x6E, 0x03, 0x91, 0x03, 0x12, 0x16, 0x97, 0xC6, 0xF0, 0xAA, 0x38, 0x7D, 0xB3,
    0x9E, 0xC7, 0x7D, 0x9D, 0xEB, 0xCE, 0x8C, 0x56, 0x0C, 0x8A, 0x77, 0x6C, 0x07, 0x17, 0x02, 0xBD,
    0x8F, 0x00, 0x94, 0x1F, 0xB4, 0x96, 0x72, 0x20, 0xBC, 0x51, 0x43, 0x8F, 0xBC, 0xA8, 0xBC, 0xBA,
    0xAF, 0x4F, 0x3C, 0x9A, 0xA2, 0x45, 0x9A, 0x14, 0x5B, 0x96, 0xDF, 0x55, 0x51, 0xC9, 0x5D, 0x68,
    0xB6, 0x5E, 0xC3, 0xFA, 0x00, 0xE2, 0x2B, 0x37, 0x34, 0x66, 0x07, 0xE0, 0xAF, 0xE6, 0x9A, 0x22
};
*/

uint8 const P[] =
{
    0x7D, 0xBD, 0xB9, 0xE1, 0x2D, 0xAE, 0x42, 0x56, 0x6E, 0x2B, 0xE2, 0x89, 0xD9, 0xBB, 0x0C, 0x1F,
    0x67, 0x28, 0xC1, 0x4D, 0x91, 0x3C, 0xAD, 0x5F, 0xF0, 0x43, 0x86, 0x5C, 0x27, 0xDC, 0x58, 0xB3,
    0x0E, 0x75, 0x77, 0x78, 0x49, 0x35, 0xE7, 0xE7, 0xDF, 0xFD, 0x74, 0xAB, 0x4E, 0xFE, 0xD3, 0xAB,
    0x6B, 0x96, 0xF7, 0x89, 0xB2, 0x5A, 0x6A, 0x25, 0x03, 0x5A, 0x92, 0x1A, 0xF1, 0xFC, 0x05, 0x4E,
    0xCE, 0xDD, 0x37, 0xA4, 0x02, 0x53, 0x76, 0xCB, 0xC2, 0xD9, 0x63, 0xCB, 0x51, 0x94, 0xEC, 0x5C,
    0x39, 0xCC, 0xB2, 0x17, 0x0C, 0xA3, 0x43, 0x9A, 0xD0, 0x83, 0x27, 0x67, 0x52, 0x64, 0x37, 0x0E,
    0x38, 0xB7, 0x9B, 0xF4, 0x2D, 0xB8, 0x0F, 0x30, 0x72, 0xD3, 0x15, 0xF3, 0x2C, 0x39, 0x55, 0x72,
    0x2C, 0x55, 0x80, 0x63, 0xA0, 0xA1, 0x6F, 0x28, 0xF3, 0xF3, 0x5A, 0x6F, 0x68, 0x59, 0xB3, 0xF3
};

uint8 const Q[] =
{
    0x0B, 0x1A, 0x13, 0x07, 0x12, 0xEF, 0xDD, 0x97, 0x01, 0x9A, 0x21, 0x7D, 0xFA, 0xA3, 0xB7, 0xE2,
    0x39, 0x2E, 0x04, 0x92, 0x96, 0x45, 0x2A, 0xEB, 0x57, 0x03, 0xAC, 0xB1, 0x83, 0xCD, 0x25, 0x4F,
    0x2C, 0xA9, 0xA1, 0x54, 0x26, 0x54, 0xCF, 0xE6, 0x1B, 0x53, 0x51, 0x3A, 0xC1, 0x15, 0xF4, 0x17,
    0xBB, 0x17, 0x1F, 0x37, 0x66, 0x36, 0x1A, 0xD4, 0xB1, 0x5B, 0x49, 0xA8, 0xF1, 0x02, 0xB0, 0x42,
    0xA9, 0x66, 0xA0, 0xE2, 0x52, 0x2C, 0x8C, 0x89, 0xA2, 0xDD, 0xA6, 0xF1, 0xA3, 0xDF, 0xB6, 0x80,
    0x63, 0xB8, 0x10, 0xDA, 0xDE, 0x84, 0x56, 0xFA, 0xFB, 0x72, 0x65, 0x5E, 0xA3, 0x9C, 0x78, 0x65,
    0xD0, 0x73, 0x07, 0x34, 0x1D, 0xE1, 0x4D, 0x77, 0xE8, 0x00, 0x0F, 0x80, 0x1C, 0x5A, 0x21, 0x55,
    0x0A, 0x8C, 0xF4, 0x93, 0xF5, 0xF8, 0x40, 0xF2, 0x40, 0xEA, 0x52, 0x12, 0x40, 0xF0, 0xBF, 0xFA
};

uint8 const DP[] =
{
    0xE1, 0xA6, 0x22, 0xAB, 0xFF, 0x57, 0x83, 0x45, 0x3F, 0x93, 0x76, 0xC8, 0xFA, 0xD9, 0x17, 0xE1,
    0x49, 0x73, 0xC2, 0x13, 0x28, 0x0B, 0x1F, 0xE2, 0x9A, 0xF4, 0x7F, 0x7C, 0x37, 0x56, 0xA1, 0xDF,
    0x51, 0x97, 0x2F, 0x15, 0x10, 0x97, 0xCD, 0x2A, 0x40, 0x09, 0xFC, 0x0A, 0xC3, 0x3F, 0x88, 0x86,
    0xA9, 0x51, 0x13, 0xE1, 0x76, 0xCF, 0xA8, 0x37, 0x9A, 0x91, 0x3B, 0xD0, 0x70, 0xA1, 0xD7, 0x03,
    0x71, 0x59, 0x6C, 0xB3, 0x41, 0xB8, 0x32, 0x68, 0x56, 0xC8, 0xB8, 0xD1, 0xF9, 0x1D, 0x04, 0xC5,
    0x13, 0xB5, 0x8E, 0x57, 0x73, 0x02, 0x97, 0x7B, 0x33, 0x60, 0x68, 0xA9, 0xC2, 0x40, 0x96, 0x3C,
    0x57, 0x4E, 0x4F, 0xC0, 0xAB, 0x21, 0x5C, 0xBA, 0x7D, 0x65, 0xAA, 0x1B, 0xD6, 0x43, 0x06, 0xCE,
    0x3E, 0x0C, 0xB9, 0xB2, 0x82, 0xB0, 0xC9, 0x54, 0x59, 0x32, 0xC5, 0x88, 0x08, 0x9C, 0x9B, 0xBF
};

uint8 const DQ[] =
{
    0xE3, 0xB1, 0xED, 0x52, 0xEF, 0xE6, 0x88, 0x40, 0x50, 0x89, 0x4C, 0x99, 0xE5, 0xF7, 0xED, 0x03,
    0x1C, 0x54, 0x11, 0x24, 0x2F, 0x9D, 0xE8, 0xE6, 0x39, 0xFA, 0x19, 0xF4, 0x06, 0x55, 0x0B, 0x8B,
    0x95, 0xC8, 0xB1, 0xE2, 0x7C, 0x75, 0x3B, 0x2A, 0x40, 0xC3, 0xE7, 0xE0, 0x25, 0x18, 0xBF, 0xB5,
    0x03, 0x1B, 0x5A, 0x57, 0x92, 0x3C, 0x85, 0x7D, 0x7F, 0x43, 0x56, 0x1F, 0x1E, 0x80, 0xC3, 0xBA,
    0xF0, 0x53, 0xD7, 0x6A, 0xD0, 0xF2, 0xDD, 0x9C, 0xC6, 0x53, 0xE7, 0xB4, 0xD3, 0x9D, 0xAB, 0xBF,
    0xE0, 0x97, 0x50, 0x92, 0x23, 0xB9, 0xB7, 0xDC, 0xAA, 0xC4, 0x20, 0x93, 0x5A, 0xF5, 0xDE, 0x76,
    0x28, 0x93, 0x91, 0x44, 0x1E, 0x4C, 0x15, 0x2F, 0x7F, 0x45, 0x3C, 0x3B, 0x7D, 0x36, 0x3B, 0x24,
    0xC7, 0x8C, 0x65, 0x43, 0xAE, 0x65, 0x84, 0xBC, 0xF9, 0x76, 0x4E, 0x3C, 0x44, 0x05, 0xBC, 0xFA
};

uint8 const InverseQ[] =
{
    0x63, 0xC1, 0x14, 0x2B, 0x57, 0x0B, 0x8A, 0x3C, 0x27, 0xDB, 0x96, 0x82, 0x27, 0xEB, 0xF6, 0x45,
    0x6D, 0x07, 0x50, 0xE8, 0x4A, 0xD4, 0xB6, 0x7A, 0x3C, 0x8B, 0x4D, 0x65, 0xF0, 0x50, 0x70, 0x84,
    0x71, 0x2B, 0xC6, 0x6D, 0x28, 0x2D, 0x76, 0x38, 0x73, 0x93, 0xDB, 0x44, 0xD7, 0xC0, 0x7F, 0xD9,
    0x57, 0x18, 0x28, 0x57, 0xF1, 0x13, 0x38, 0xA4, 0x91, 0x67, 0x1E, 0x13, 0x73, 0x55, 0xFC, 0x7B,
    0xAF, 0x50, 0xFA, 0xFD, 0x16, 0x12, 0x6F, 0xA4, 0x95, 0x15, 0x9C, 0x07, 0x18, 0xA6, 0x46, 0xFD,
    0xB3, 0xCF, 0xA5, 0x0E, 0x05, 0x30, 0xEC, 0x2C, 0xCD, 0x62, 0xDD, 0x6F, 0xB1, 0xFE, 0x6C, 0x05,
    0x2F, 0x11, 0xA6, 0xA0, 0x98, 0xAC, 0x9B, 0x15, 0xF0, 0x04, 0xC4, 0x7B, 0x79, 0xAA, 0x51, 0x25,
    0x2A, 0x84, 0x73, 0xE6, 0x77, 0x47, 0xA3, 0xEB, 0xCF, 0x6D, 0xC8, 0x96, 0x3A, 0x1B, 0x02, 0x52
};

uint8 const WherePacketHmac[] =
{
    0x2C, 0x1F, 0x1D, 0x80, 0xC3, 0x8C, 0x23, 0x64, 0xDA, 0x90, 0xCA, 0x8E, 0x2C, 0xFC, 0x0C, 0xCE,
    0x09, 0xD3, 0x62, 0xF9, 0xF3, 0x8B, 0xBE, 0x9F, 0x19, 0xEF, 0x58, 0xA1, 0x1C, 0x34, 0x14, 0x41,
    0x3F, 0x23, 0xFD, 0xD3, 0xE8, 0x14, 0xEC, 0x2A, 0xFD, 0x4F, 0x95, 0xBA, 0x30, 0x7E, 0x56, 0x5D,
    0x83, 0x95, 0x81, 0x69, 0xB0, 0x5A, 0xB4, 0x9D, 0xA8, 0x55, 0xFF, 0xFC, 0xEE, 0x58, 0x0A, 0x2F
};

WorldPackets::Auth::ConnectTo::ConnectTo() : ServerPacket(SMSG_REDIRECT_CLIENT, 8 + 4 + 256 + 1)
{
    HexStrToByteArray("F41DCB2D728CF3337A4FF338FA89DB01BBBE9C3B65E9DA96268687353E48B94C", Payload.PanamaKey);
    Payload.Adler32 = 0xA0A66C10;

    p.SetBinary(P, 128);
    q.SetBinary(Q, 128);
    dmp1.SetBinary(DP, 128);
    dmq1.SetBinary(DQ, 128);
    iqmp.SetBinary(InverseQ, 128);
}

WorldPacket const* WorldPackets::Auth::ConnectTo::Write()
{
    ByteBuffer payload;
    uint16 port = Payload.Where.port();
    uint8 address[16] = { 0 };
    uint32 addressType = 3;
    if (Payload.Where.address().is_v4())
    {
        memcpy(address, Payload.Where.address().to_v4().to_bytes().data(), 4);
        addressType = 1;
    }
    else
    {
        memcpy(address, Payload.Where.address().to_v6().to_bytes().data(), 16);
        addressType = 2;
    }

    HmacSha1 hmacHash(64, WherePacketHmac);
    hmacHash.UpdateData(address, 16);
    hmacHash.UpdateData((uint8* const)&addressType, 4);
    hmacHash.UpdateData((uint8* const)&port, 2);
    hmacHash.UpdateData((uint8* const)Haiku.c_str(), 71);
    hmacHash.UpdateData(Payload.PanamaKey, 32);
    hmacHash.UpdateData(PiDigits, 108);
    hmacHash.UpdateData(&Payload.XorMagic, 1);
    hmacHash.Finalize();

    uint8* hmac = hmacHash.GetDigest();

    payload << uint8(PiDigits[10]);
    payload << uint8(Haiku[66]);
    payload << uint8(Haiku[12]);
    payload << uint8(PiDigits[89]);
    payload << uint8(PiDigits[50]);
    payload << uint8(Haiku[48]);
    payload << uint8(PiDigits[32]);
    payload << uint8(PiDigits[0]);
    payload << uint8(Payload.PanamaKey[22]);
    payload << uint8(PiDigits[90]);
    payload << uint8(Payload.PanamaKey[16]);
    payload << uint8(PiDigits[69]);
    payload << uint8(Haiku[39]);
    payload << uint8(PiDigits[107]);
    payload << uint8(address[7]);
    payload << uint8(hmac[2]);
    payload << uint8(PiDigits[55]);
    payload << uint8(Haiku[13]);
    payload << uint8(PiDigits[34]);
    payload << uint8(Haiku[51]);
    payload << uint8(PiDigits[37]);
    payload << uint8(hmac[11]);
    payload << uint8(address[8]);
    payload << uint8(Haiku[10]);
    payload << uint8(Haiku[47]);
    payload << uint8(Haiku[29]);
    payload << uint8(PiDigits[93]);
    payload << uint8(hmac[5]);
    payload << uint8(hmac[14]);
    payload << uint8(PiDigits[71]);
    payload << uint8(PiDigits[3]);
    payload << uint8(PiDigits[103]);
    payload << uint8(PiDigits[80]);
    payload << uint8(Haiku[59]);
    payload << uint8(Haiku[61]);
    payload << uint8(PiDigits[1]);
    payload << uint8(Haiku[69]);
    payload << uint8(Payload.PanamaKey[11]);
    payload << uint8(Haiku[45]);
    payload << uint8(PiDigits[52]);
    payload << uint8(PiDigits[43]);
    payload << uint8(PiDigits[81]);
    payload << uint8(Payload.PanamaKey[4]);
    payload << uint8(Payload.PanamaKey[10]);
    payload << uint8(Haiku[37]);
    payload << uint8(Haiku[56]);
    payload << uint8(hmac[12]);
    payload << uint8(PiDigits[97]);
    payload << uint8(Haiku[32]);
    payload << uint8(PiDigits[17]);
    payload << uint8(Payload.XorMagic);
    payload << uint8(PiDigits[26]);
    payload << uint8(PiDigits[47]);
    payload << uint8(Haiku[60]);
    payload << uint8(Haiku[2]);
    payload << uint8(Haiku[1]);
    payload << uint8(hmac[3]);
    payload << uint8(PiDigits[64]);
    payload << uint8(PiDigits[18]);
    payload << uint8(Haiku[53]);
    payload << uint8(PiDigits[79]);
    payload << uint8(Payload.PanamaKey[29]);
    payload << uint8(Haiku[43]);
    payload << uint8(PiDigits[104]);
    payload << uint8(PiDigits[56]);
    payload << uint8(Payload.PanamaKey[31]);
    payload << uint8(hmac[10]);
    payload << uint8(PiDigits[94]);
    payload << uint8(PiDigits[22]);
    payload << uint8(hmac[8]);
    payload << uint8(PiDigits[77]);
    payload << uint8(address[13]);
    payload << uint8(PiDigits[36]);
    payload << uint8(PiDigits[101]);
    payload << uint8(PiDigits[6]);
    payload << uint8(PiDigits[78]);
    payload << uint8(hmac[15]);
    payload << uint8(PiDigits[88]);
    payload << uint8(PiDigits[59]);
    payload << uint8(PiDigits[67]);
    payload << uint8(Payload.PanamaKey[1]);
    payload << uint8(Payload.PanamaKey[30]);
    payload << uint8(PiDigits[95]);
    payload << uint8(PiDigits[4]);
    payload << uint8(Payload.PanamaKey[15]);
    payload << uint8(Haiku[64]);
    payload << uint8(PiDigits[86]);
    payload << uint8(Haiku[35]);
    payload << uint8(address[10]);
    payload << uint8(Payload.PanamaKey[5]);
    payload << uint8(PiDigits[74]);
    payload << uint8(PiDigits[60]);
    payload << uint8(Haiku[40]);
    payload << uint8(PiDigits[105]);
    payload << uint8(Payload.PanamaKey[25]);
    payload << uint8(Haiku[57]);
    payload << uint8(PiDigits[84]);
    payload << uint8(PiDigits[70]);
    payload << uint8(PiDigits[23]);
    payload << uint8(Haiku[11]);
    payload << uint8(hmac[16]);
    payload << uint8(PiDigits[57]);
    payload << uint8(Haiku[6]);
    payload << uint8(Haiku[8]);
    payload << uint8(Haiku[65]);
    payload << uint8(Haiku[28]);
    payload << uint8(Payload.PanamaKey[13]);
    payload << uint8(PiDigits[91]);
    payload << uint8(PiDigits[62]);
    payload << uint8(PiDigits[7]);
    payload << uint8(PiDigits[40]);
    payload << uint8(Haiku[23]);
    payload << uint8(PiDigits[41]);
    payload << uint8(Payload.PanamaKey[28]);
    payload << uint8(PiDigits[25]);
    payload << uint8(PiDigits[38]);
    payload << uint32(Payload.Adler32);
    payload << uint8(Haiku[30]);
    payload << uint8(PiDigits[15]);
    payload << uint8(hmac[18]);
    payload << uint8(PiDigits[13]);
    payload << uint8(PiDigits[27]);
    payload << uint8(Haiku[52]);
    payload << uint8(PiDigits[68]);
    payload << uint8(Haiku[58]);
    payload << uint8(Haiku[34]);
    payload << uint8(PiDigits[87]);
    payload << uint8(PiDigits[72]);
    payload << uint8(Haiku[42]);
    payload << uint8(PiDigits[24]);
    payload << uint8(hmac[1]);
    payload << uint8(Haiku[18]);
    payload << uint8(Haiku[25]);
    payload << uint8(Payload.PanamaKey[24]);
    payload << uint8(Haiku[27]);
    payload << uint8(hmac[9]);
    payload << uint8(hmac[4]);
    payload << uint8(Haiku[26]);
    payload << uint8(PiDigits[45]);
    payload << uint8(Haiku[9]);
    payload << uint8(address[6]);
    payload << uint8(PiDigits[73]);
    payload << uint8(Haiku[20]);
    payload << uint8(Haiku[67]);
    payload << uint8(Payload.PanamaKey[27]);
    payload << uint8(address[1]);
    payload << uint8(PiDigits[33]);
    payload << uint8(hmac[0]);
    payload << uint8(Haiku[3]);
    payload << uint8(PiDigits[54]);
    payload << uint8(hmac[17]);
    payload << uint8(PiDigits[35]);
    payload << uint8(address[4]);
    payload << uint8(Haiku[46]);
    payload << uint8(Payload.PanamaKey[2]);
    payload << uint8(address[12]);
    payload << uint8(Haiku[68]);
    payload << uint8(Haiku[24]);
    payload << uint8(PiDigits[48]);
    payload << uint8(port & 0xFF);
    payload << uint8(Haiku[14]);
    payload << uint8(Payload.PanamaKey[12]);
    payload << uint8(Haiku[38]);
    payload << uint8(PiDigits[53]);
    payload << uint8(PiDigits[49]);
    payload << uint8(Haiku[4]);
    payload << uint8(PiDigits[63]);
    payload << uint8((port >> 8) & 0xFF);
    payload << uint8(Haiku[0]);
    payload << uint8(PiDigits[76]);
    payload << uint8(PiDigits[100]);
    payload << uint8(Payload.PanamaKey[14]);
    payload << uint8(Payload.PanamaKey[6]);
    payload << uint8(Haiku[16]);
    payload << uint8(PiDigits[65]);
    payload << uint8(PiDigits[14]);
    payload << uint8(Haiku[19]);
    payload << uint8(PiDigits[66]);
    payload << uint8(PiDigits[28]);
    payload << uint8(Payload.PanamaKey[18]);
    payload << uint8(PiDigits[102]);
    payload << uint8(PiDigits[51]);
    payload << uint8(Haiku[63]);
    payload << uint8(address[2]);
    payload << uint8(hmac[6]);
    payload << uint8(Haiku[21]);
    payload << uint8(Haiku[15]);
    payload << uint8(Payload.PanamaKey[21]);
    payload << uint8(Haiku[41]);
    payload << uint8(Haiku[5]);
    payload << uint8(Payload.PanamaKey[7]);
    payload << uint8(Payload.PanamaKey[20]);
    payload << uint8(PiDigits[46]);
    payload << uint8(PiDigits[44]);
    payload << uint8(PiDigits[96]);
    payload << uint8(PiDigits[99]);
    payload << uint8(hmac[13]);
    payload << uint8(Haiku[70]);
    payload << uint8(addressType);
    payload << uint8(Payload.PanamaKey[17]);
    payload << uint8(Payload.PanamaKey[23]);
    payload << uint8(PiDigits[58]);
    payload << uint8(PiDigits[2]);
    payload << uint8(PiDigits[61]);
    payload << uint8(PiDigits[19]);
    payload << uint8(PiDigits[83]);
    payload << uint8(PiDigits[42]);
    payload << uint8(PiDigits[29]);
    payload << uint8(Payload.PanamaKey[26]);
    payload << uint8(PiDigits[5]);
    payload << uint8(PiDigits[85]);
    payload << uint8(Haiku[50]);
    payload << uint8(hmac[19]);
    payload << uint8(Haiku[31]);
    payload << uint8(address[14]);
    payload << uint8(Payload.PanamaKey[0]);
    payload << uint8(PiDigits[98]);
    payload << uint8(Haiku[17]);
    payload << uint8(Haiku[55]);
    payload << uint8(Haiku[54]);
    payload << uint8(PiDigits[20]);
    payload << uint8(PiDigits[21]);
    payload << uint8(Payload.PanamaKey[3]);
    payload << uint8(Payload.PanamaKey[19]);
    payload << uint8(address[15]);
    payload << uint8(PiDigits[12]);
    payload << uint8(Haiku[49]);
    payload << uint8(PiDigits[9]);
    payload << uint8(hmac[7]);
    payload << uint8(Payload.PanamaKey[9]);
    payload << uint8(PiDigits[8]);
    payload << uint8(PiDigits[11]);
    payload << uint8(address[9]);
    payload << uint8(Haiku[44]);
    payload << uint8(address[3]);
    payload << uint8(Haiku[33]);
    payload << uint8(address[11]);
    payload << uint8(Haiku[22]);
    payload << uint8(address[5]);
    payload << uint8(Haiku[36]);
    payload << uint8(PiDigits[82]);
    payload << uint8(PiDigits[16]);
    payload << uint8(PiDigits[75]);
    payload << uint8(Haiku[7]);
    payload << uint8(PiDigits[92]);
    payload << uint8(Payload.PanamaKey[8]);
    payload << uint8(Haiku[62]);
    payload << uint8(PiDigits[106]);
    payload << uint8(address[0]);
    payload << uint8(PiDigits[31]);
    payload << uint8(PiDigits[39]);
    payload << uint8(PiDigits[30]);

    BigNumber bnData;
    bnData.SetBinary(payload.contents(), payload.size());

    BigNumber m1 = (bnData % p).ModExp(dmp1, p);
    BigNumber m2 = (bnData % q).ModExp(dmq1, q);
    BigNumber h = (iqmp * (m1 - m2)) % p;
    // Be sure to use the positive remainder
    if (h.IsNegative())
        h += p;

    BigNumber m = m2 + h * q;

    _worldPacket << uint64(Key);
    _worldPacket << uint32(Serial);
    _worldPacket.append(m.AsByteArray(256).get(), 256);
    _worldPacket << uint8(Con);
    return &_worldPacket;
}

void WorldPackets::Auth::AuthContinuedSession::Read()
{
    _worldPacket >> DosResponse;
    _worldPacket >> Key;
    _worldPacket.read(Digest, SHA_DIGEST_LENGTH);
}

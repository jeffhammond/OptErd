/*
 * Copyright (c) 2003-2010 University of Florida
 * Copyright (c) 2013-2015 Georgia Institute of Technology
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published
 * by the Free Software Foundation; either version 2.1 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * The GNU Lesser General Public License is included in this distribution
 * in the file COPYING.
 */

#include "jacobi.h"

#ifdef __INTEL_OFFLOAD
#pragma offload_attribute(push, target(mic))
#endif



/* (2i+1) / ((4i-1) * (4i+3)) for i = 1...100 */
const double r2[100] = {
	0x1.2492492492492p-3,  0x1.09F959C427E56p-4,  0x1.5B8A15B8A15B9p-5,  0x1.02B1DA46102B2p-5,
	0x1.9C69723BC0BB7p-6,  0x1.56FB77230A4E5p-6,  0x1.259EBD04967AFp-6,  0x1.00B5349D9C6E7p-6,
	0x1.C81C81C81C81Dp-7,  0x1.9A552E4107C91p-7,  0x1.74EA97C02888Dp-7,  0x1.55C2B25B35146p-7,
	0x1.3B69F598813B7p-7,  0x1.24D78774857BAp-7,  0x1.11497CBA81E4Ep-7,  0x1.002E941AED9F9p-7,
	0x1.E22FAE3357050p-8,  0x1.C75E153FCBB1Cp-8,  0x1.AF604ECF22171p-8,  0x1.99C9933A723CDp-8,
	0x1.8641DEBEF1FF1p-8,  0x1.7481366BAD59Cp-8,  0x1.644C29D01644Cp-8,  0x1.557134C75B288p-8,
	0x1.47C6C277739C5p-8,  0x1.3B29A603642F2p-8,  0x1.2F7BEA5C0965Dp-8,  0x1.24A3E35C97DAFp-8,
	0x1.1A8B7068752DAp-8,  0x1.111F64F825A37p-8,  0x1.084F0E80E8D7Fp-8,  0x1.000BD148ACB54p-8,
	0x1.F091AE8F82B32p-9,  0x1.E1F59ABEF7724p-9,  0x1.D42F580E1D98Bp-9,  0x1.C72D1266C2AEBp-9,
	0x1.BADEE374A2570p-9,  0x1.AF3691A05673Ep-9,  0x1.A427590D53C12p-9,  0x1.99A5BCD9FB14Bp-9,
	0x1.8FA75F38795B1p-9,  0x1.8622DF3C12407p-9,  0x1.7D0FBB6CE49DDp-9,  0x1.7466385F84D36p-9,
	0x1.6C1F4AB06B293p-9,  0x1.643483DF0DFB5p-9,  0x1.5CA0019B02B3Cp-9,  0x1.555C5F27CA470p-9,
	0x1.4E64A88ADB029p-9,  0x1.47B44F43B642Fp-9,  0x1.41472057EC29Fp-9,  0x1.3B193B85442BFp-9,
	0x1.35270B732B9DAp-9,  0x1.2F6D3EC24E7FCp-9,  0x1.29E8C1DE1052Cp-9,  0x1.2496B977A5E6Bp-9,
	0x1.1F747D95F8BEDp-9,  0x1.1A7F95285CEBDp-9,  0x1.15B5B20C922A8p-9,  0x1.1114AD7A9B5CCp-9,
	0x1.0C9A84CABB745p-9,  0x1.0845568B69400p-9,  0x1.04135FDE56B07p-9,  0x1.0002FA14C4B4Ep-9,
	0x1.F8253108A3E3Bp-10, 0x1.F0818D1C8C883p-10, 0x1.E9184BA82880Dp-10, 0x1.E1E6D928D1B49p-10,
	0x1.DAEAC85957304p-10, 0x1.D421CF76945F7p-10, 0x1.CD89C5BF27265p-10, 0x1.C720A12884FB9p-10,
	0x1.C0E47444507B4p-10, 0x1.BAD36C515EA5Ep-10, 0x1.B4EBCF7457B2Dp-10, 0x1.AF2BFB144D4FAp-10,
	0x1.A992625801390p-10, 0x1.A41D8CC0ECFBEp-10, 0x1.9ECC14E167C4Ap-10, 0x1.999CA72B8AA8Fp-10,
	0x1.948E00D6AFDF7p-10, 0x1.8F9EEED99EC6Ep-10, 0x1.8ACE4CF7A641Ap-10, 0x1.861B04DF11733p-10,
	0x1.81840D5788F3Fp-10, 0x1.7D08697F149B9p-10, 0x1.78A7281490884p-10, 0x1.745F62CE83714p-10,
	0x1.70303DBD5CF1Fp-10, 0x1.6C18E6B838A79p-10, 0x1.681894D356F39p-10, 0x1.642E87DF8E39Fp-10,
	0x1.605A07F207B98p-10, 0x1.5C9A64F3A9D26p-10, 0x1.58EEF6379EE38p-10, 0x1.55571A1873F7Cp-10,
	0x1.51D2359B5571Fp-10, 0x1.4E5FB418F9D6Cp-10, 0x1.4AFF06EBD3E09p-10, 0x1.47AFA5232D413p-10
};

/* ((4i-3) * (4i+1) * square(4i-1)) / (2i * (2i+1) * square(2i-1)) for i = 1...100 */
const double sinv[100] = {
	0x1.E000000000000p+2, 0x1.8800000000000p+3, 0x1.AF7390D2A6C40p+3, 0x1.C305397829CBCp+3,
	0x1.CEDBA0269D90Ap+3, 0x1.D6D1F15A53003p+3, 0x1.DC8D179665B52p+3, 0x1.E0E07A13AD46Ep+3,
	0x1.E44253C3841C5p+3, 0x1.E6FA103FDCFD2p+3, 0x1.E9356C055E797p+3, 0x1.EB13112195BA8p+3,
	0x1.ECA855C8F2102p+3, 0x1.EE048926E579Dp+3, 0x1.EF32F0D6440E5p+3, 0x1.F03C08D0A7C6Ep+3,
	0x1.F126535C1FFA8p+3, 0x1.F1F6E4397B898p+3, 0x1.F2B1C039B5166p+3, 0x1.F35A20612EE64p+3,
	0x1.F3F2A202F7430p+3, 0x1.F47D69D758588p+3, 0x1.F4FC3DFCEE79Ap+3, 0x1.F5709985DCA45p+3,
	0x1.F5DBBB5C829DFp+3, 0x1.F63EB1BE9E296p+3, 0x1.F69A632FD6BE8p+3, 0x1.F6EF9584CD996p+3,
	0x1.F73EF37B9EA9Dp+3, 0x1.F7891137BFBE7p+3, 0x1.F7CE6FE11F543p+3, 0x1.F80F8096834FDp+3,
	0x1.F84CA6D795974p+3, 0x1.F8863A828631Ap+3, 0x1.F8BC897AD2947p+3, 0x1.F8EFD909FDAC9p+3,
	0x1.F9206707676B7p+3, 0x1.F94E6AD1AFC7Dp+3, 0x1.F97A1621F0A08p+3, 0x1.F9A395BF7358Bp+3,
	0x1.F9CB12193CC4Bp+3, 0x1.F9F0AFC9C62CCp+3, 0x1.FA1490086C7DAp+3, 0x1.FA36D10B7AF01p+3,
	0x1.FA578E5D33430p+3, 0x1.FA76E125CC015p+3, 0x1.FA94E06C07CF5p+3, 0x1.FAB1A14DC3354p+3,
	0x1.FACD37319D8BDp+3, 0x1.FAE7B3F2B30F7p+3, 0x1.FB01280737B17p+3, 0x1.FB19A2A2A26E5p+3,
	0x1.FB3131D3FE840p+3, 0x1.FB47E2A0E1D9Ap+3, 0x1.FB5DC11D75759p+3, 0x1.FB72D881ED640p+3,
	0x1.FB87333DC05DCp+3, 0x1.FB9ADB08E47A0p+3, 0x1.FBADD8F34CD51p+3, 0x1.FBC03572DC23Cp+3,
	0x1.FBD1F86FF95F4p+3, 0x1.FBE32950EDE64p+3, 0x1.FBF3CF042F74Fp+3, 0x1.FC03F009B40EAp+3,
	0x1.FC13927B6A3FDp+3, 0x1.FC22BC14ECF12p+3, 0x1.FC31723A873F7p+3, 0x1.FC3FB9FF9A672p+3,
	0x1.FC4D982C75BA8p+3, 0x1.FC5B1143AECA5p+3, 0x1.FC682987064EFp+3, 0x1.FC74E4FBE4FE3p+3,
	0x1.FC81476F7A40Fp+3, 0x1.FC8D547A85A77p+3, 0x1.FC990F84D30DBp+3, 0x1.FCA47BC870865p+3,
	0x1.FCAF9C54A46C6p+3, 0x1.FCBA7410A9504p+3, 0x1.FCC505BE34E48p+3, 0x1.FCCF53FBCE892p+3,
	0x1.FCD96146F9A19p+3, 0x1.FCE32FFE37787p+3, 0x1.FCECC262E4178p+3, 0x1.FCF61A9AF128Fp+3,
	0x1.FCFF3AB281AACp+3, 0x1.FD08249D68FDBp+3, 0x1.FD10DA388F9A9p+3, 0x1.FD195D4B3F86Ap+3,
	0x1.FD21AF885A6CDp+3, 0x1.FD29D28F7B18Ep+3, 0x1.FD31C7EE03E86p+3, 0x1.FD3991201BA47p+3,
	0x1.FD412F919A15Ap+3, 0x1.FD48A49EE587Ep+3, 0x1.FD4FF195C259Ap+3, 0x1.FD5717B6159A1p+3,
	0x1.FD5E18329BA56p+3, 0x1.FD64F43193994p+3, 0x1.FD6BACCD606BCp+3, 0x1.FD724315205E3p+3
};

const double csmall[16] = {
	-0.8888888888888889e-1,  0.2902494331065760e-2,
	-0.6150655501304852e-4,  0.9697564430413280e-6,
	-0.1218995971140662e-7,  0.1274629327641546e-9,
	-0.1141203534061737e-11, 0.8934294807029541e-14,
	-0.6214538312345874e-16, 0.3889208875361482e-18,
	-0.2212173840910540e-20, 0.1153219167688572e-22,
	-0.5548593532844672e-25, 0.2478690843269028e-27,
	-0.1033382005056540e-29, 0.4038684854215214e-32
};

/* (4i * (2i+1) - 1) / ((4i+3) * (4i-1)) for i = 0...99 */
const double ajac[100] = {
	0x1.5555555555555p-2, 0x1.0C30C30C30C31p-1, 0x1.03531DEC0D4C7p-1, 0x1.018D3018D3019p-1,
	0x1.00E5F36CB00E6p-1, 0x1.0095F7CC72D1Cp-1, 0x1.006988736D3E4p-1, 0x1.004E4C76ABE3Ep-1,
	0x1.003C66DF3424Dp-1, 0x1.0030030030030p-1, 0x1.0027144D8C49Ep-1, 0x1.00206D715E9F6p-1,
	0x1.001B574177EFCp-1, 0x1.00175D2EA3001p-1, 0x1.0014322CA6EECp-1, 0x1.0011A1A4F3423p-1,
	0x1.000F86B3A48A8p-1, 0x1.000DC6D91768Ep-1, 0x1.000C4EA6A0DC0p-1, 0x1.000B0F98FEBF4p-1,
	0x1.0009FEAC2D220p-1, 0x1.000913646F9A7p-1, 0x1.0008472357B9Ep-1, 0x1.000794AF30008p-1,
	0x1.0006F7DC8174Dp-1, 0x1.00066D4F1B70Bp-1, 0x1.0005F24B95CCCp-1, 0x1.000584948D4F5p-1,
	0x1.0005225056F53p-1, 0x1.0004C9F4E365Dp-1, 0x1.00047A38366DCp-1, 0x1.000432044B306p-1,
	0x1.0003F06D8EE71p-1, 0x1.0003B4AB55217p-1, 0x1.00037E11D126Ep-1, 0x1.00034C0D3D653p-1,
	0x1.00031E1DEF2B7p-1, 0x1.0002F3D5249E2p-1, 0x1.0002CCD26671Ap-1, 0x1.0002A8C15FADEp-1,
	0x1.00028758144F0p-1, 0x1.000268556494Cp-1, 0x1.00024B7FCEA8Cp-1, 0x1.000230A463335p-1,
	0x1.00021795E3C5Cp-1, 0x1.0002002C03C85p-1, 0x1.0001EA42C6043p-1, 0x1.0001D5B9F1FEBp-1,
	0x1.0001C2749D3C7p-1, 0x1.0001B058C53A0p-1, 0x1.00019F4EF776Cp-1, 0x1.00018F420565Dp-1,
	0x1.0001801EC2769p-1, 0x1.000171D3CAB4Dp-1, 0x1.0001645150BEAp-1, 0x1.00015788F2026p-1,
	0x1.00014B6D90635p-1, 0x1.00013FF330835p-1, 0x1.0001350EDC11Ep-1, 0x1.00012AB687951p-1,
	0x1.000120E0FB37Ap-1, 0x1.00011785BE38Fp-1, 0x1.00010E9D04A75p-1, 0x1.0001061F9F1C9p-1,
	0x1.0000FE06EC3C5p-1, 0x1.0000F64CCBBDEp-1, 0x1.0000EEEB92D21p-1, 0x1.0000E7DE01BBDp-1,
	0x1.0000E11F3A780p-1, 0x1.0000DAAAB8555p-1, 0x1.0000D47C48616p-1, 0x1.0000CE9002927p-1,
	0x1.0000C8E24399Dp-1, 0x1.0000C36FA74CCp-1, 0x1.0000BE350392Ap-1, 0x1.0000B92F63CB9p-1,
	0x1.0000B45C04A27p-1, 0x1.0000AFB8503ECp-1, 0x1.0000AB41DACDDp-1, 0x1.0000A6F65F592p-1,
	0x1.0000A2D3BCE33p-1, 0x1.00009ED7F3C3Dp-1, 0x1.00009B01233D2p-1, 0x1.0000974D8744Ep-1,
	0x1.000093BB767BEp-1, 0x1.000090496050Bp-1, 0x1.00008CF5CB483p-1, 0x1.000089BF53699p-1,
	0x1.000086A4A8C9Fp-1, 0x1.000083A48E356p-1, 0x1.000080BDD7F1Fp-1, 0x1.00007DEF6A9B1p-1,
	0x1.00007B383A134p-1, 0x1.0000789748899p-1, 0x1.0000760BA5923p-1, 0x1.000073946D4EFp-1,
	0x1.00007130C7A8Ap-1, 0x1.00006EDFE7957p-1, 0x1.00006CA10A6D8p-1, 0x1.00006A73774A9p-1
};

/* (4*square(i) * (4*square(i) - 4*i + 1)) / ((4i-3) * (4i+1) * square(4i-1)) for i = 1...99 */
const double bjac[99] = {
	0x1.6C16C16C16C17p-4, 0x1.0B7E6EC259DC8p-4, 0x1.0464E7198D19Ep-4, 0x1.02526768B8CDDp-4,
	0x1.016FA82F87C49p-4, 0x1.00F9E59CFEEE1p-4, 0x1.00B4E9B7F8C3Fp-4, 0x1.008908584C2DDp-4,
	0x1.006B6452F6180p-4, 0x1.00566E3610FB5p-4, 0x1.00470F9A1BF70p-4, 0x1.003B74DFB78A8p-4,
	0x1.00327AF157606p-4, 0x1.002B64E5A9631p-4, 0x1.0025B3C89B75Ap-4, 0x1.00210FB8EB097p-4,
	0x1.001D3A4FE07D8p-4, 0x1.001A064059CE5p-4, 0x1.00175202082FAp-4, 0x1.00150454900F2p-4,
	0x1.001309E8D9540p-4, 0x1.001153C6C30C7p-4, 0x1.000FD62E64AE5p-4, 0x1.000E87CC186C8p-4,
	0x1.000D612512D21p-4, 0x1.000C5C2B4ACE2p-4, 0x1.000B73ED270E5p-4, 0x1.000AA45913201p-4,
	0x1.0009EA0F8A737p-4, 0x1.0009423FC6201p-4, 0x1.0008AA8C54518p-4, 0x1.000820F5A2D0Fp-4,
	0x1.0007A3C90E2C6p-4, 0x1.00073193674CFp-4, 0x1.0006C916264A6p-4, 0x1.0006693EB33EDp-4,
	0x1.0006111F5269Ep-4, 0x1.0005BFE95BEE4p-4, 0x1.000574E87B91Cp-4, 0x1.00052F7EC4065p-4,
	0x1.0004EF216CB4Ap-4, 0x1.0004B35619BE8p-4, 0x1.00047BB094A83p-4, 0x1.000447D0E14A9p-4,
	0x1.000417619EBC8p-4, 0x1.0003EA16A7106p-4, 0x1.0003BFABE341Ap-4, 0x1.000397E44AAB9p-4,
	0x1.0003728906F69p-4, 0x1.00034F68B6ADEp-4, 0x1.00032E56C9B4Cp-4, 0x1.00030F2AF3A4Bp-4,
	0x1.0002F1C0B0C9Fp-4, 0x1.0002D5F6DAFFBp-4, 0x1.0002BBAF4C1F7p-4, 0x1.0002A2CE8C153p-4,
	0x1.00028B3B88F7Dp-4, 0x1.000274DF57C59p-4, 0x1.00025FA4FC9A4p-4, 0x1.00024B793960Ap-4,
	0x1.0002384A62264p-4, 0x1.000226083658Dp-4, 0x1.000214A3BE4D7p-4, 0x1.0002040F2C89Ap-4,
	0x1.0001F43DC257Cp-4, 0x1.0001E523B7411p-4, 0x1.0001D6B623156p-4, 0x1.0001C8EAEA33Ap-4,
	0x1.0001BBB8ABD07p-4, 0x1.0001AF16B200Fp-4, 0x1.0001A2FCE3573p-4, 0x1.00019763B5E31p-4,
	0x1.00018C4423715p-4, 0x1.000181979EE67p-4, 0x1.000177580A96Bp-4, 0x1.00016D7FAF816p-4,
	0x1.0001640935577p-4, 0x1.00015AEF9B38Bp-4, 0x1.0001522E3114Ep-4, 0x1.000149C091A17p-4,
	0x1.000141A29CD36p-4, 0x1.000139D072D29p-4, 0x1.000132466F58Cp-4, 0x1.00012B0125740p-4,
	0x1.000123FD5BA15p-4, 0x1.00011D380838Fp-4, 0x1.000116AE4E23Ep-4, 0x1.0001105D79D41p-4,
	0x1.00010A42FE79Ap-4, 0x1.0001045C736F7p-4, 0x1.0000FEA791DA7p-4, 0x1.0000F9223276Fp-4,
	0x1.0000F3CA4B903p-4, 0x1.0000EE9DEF1EDp-4, 0x1.0000E99B490A6p-4, 0x1.0000E4C09D8B8p-4,
	0x1.0000E00C47ABFp-4, 0x1.0000DB7CB7E1Ap-4, 0x1.0000D71072C37p-4
};

#ifdef __INTEL_OFFLOAD
#pragma offload_attribute(pop)
#endif

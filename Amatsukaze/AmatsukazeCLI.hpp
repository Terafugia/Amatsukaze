/**
* Amtasukaze Command Line Interface
* Copyright (c) 2017 Nekopanda
*
* This software is released under the MIT License.
* http://opensource.org/licenses/mit-license.php
*/
#pragma once

#include <time.h>

#include "TranscodeManager.hpp"

// MSVC�̃}���`�o�C�g��Unicode�łȂ��̂ŕ����񑀍�ɓK���Ȃ��̂�wchar_t�ŕ����񑀍������
#ifdef _MSC_VER
namespace std { typedef wstring tstring; }
typedef wchar_t tchar;
#define PRITSTR "ls"
#define _T(s) L ## s
#else
namespace std { typedef string tstring; }
typedef char tchar;
#define PRITSTR "s"
#define _T(s) s
#endif

static std::string to_string(std::wstring str) {
	if (str.size() == 0) {
		return std::string();
	}
	int dstlen = WideCharToMultiByte(
		CP_ACP, 0, str.c_str(), (int)str.size(), NULL, 0, NULL, NULL);
	std::vector<char> ret(dstlen);
	WideCharToMultiByte(CP_ACP, 0,
		str.c_str(), (int)str.size(), ret.data(), (int)ret.size(), NULL, NULL);
	return std::string(ret.begin(), ret.end());
}

static std::string to_string(std::string str) {
	return str;
}

static void printCopyright() {
	printf(
		"Amatsukaze - Automated MPEG2-TS Transcode Utility\n"
		"Built on %s %s\n"
		"Copyright (c) 2017 Nekopanda\n", __DATE__, __TIME__);
}

static void printHelp(const tchar* bin) {
	printf(
		"%" PRITSTR " <�I�v�V����> -i <input.ts> -o <output.mp4>\n"
		"�I�v�V���� []�̓f�t�H���g�l \n"
		"  -i|--input  <�p�X>  ����TS�t�@�C���p�X\n"
		"  -o|--output <�p�X>  �o��MP4�t�@�C���p�X\n"
		"  -s|--serviceid <���l> ��������T�[�r�XID���w��[]\n"
		"  -w|--work   <�p�X>  �ꎞ�t�@�C���p�X[./]\n"
		"  -et|--encoder-type <�^�C�v>  �g�p�G���R�[�_�^�C�v[x264]\n"
		"                      �Ή��G���R�[�_: x264,x265,QSVEnc\n"
		"  -e|--encoder <�p�X> �G���R�[�_�p�X[x264.exe]\n"
		"  -eo|--encoder-opotion <�I�v�V����> �G���R�[�_�֓n���I�v�V����[--crf 23]\n"
		"                      ���̓t�@�C���̉𑜓x�A�A�X�y�N�g��A�C���^���[�X�t���O�A\n"
		"                      �t���[�����[�g�A�J���[�}�g���N�X���͎����Œǉ������̂ŕs�v\n"
		"  -m|--muxer  <�p�X>  L-SMASH��muxer�ւ̃p�X[muxer.exe]\n"
		"  -t|--timelineeditor <�p�X> L-SMASH��timelineeditor�ւ̃p�X[timelineeditor.exe]\n"
		"  -j|--json   <�p�X>  �o�͌��ʏ���JSON�o�͂���ꍇ�͏o�̓t�@�C���p�X���w��[]\n"
		"  --dump              �����r���̃f�[�^���_���v�i�f�o�b�O�p�j\n",
		bin);
}

static std::tstring getParam(int argc, tchar* argv[], int ikey) {
	if (ikey + 1 >= argc) {
		THROWF(FormatException,
			"%" PRITSTR "�I�v�V�����̓p�����[�^���K�v�ł�", argv[ikey]);
	}
	return argv[ikey + 1];
}

static std::tstring pathNormalize(std::tstring path) {
	if (path.size() != 0) {
		// �o�b�N�X���b�V���̓X���b�V���ɕϊ�
		std::replace(path.begin(), path.end(), _T('\\'), _T('/'));
		// �Ō�̃X���b�V���͎��
		if (path.back() == _T('/')) {
			path.pop_back();
		}
	}
	return path;
}

static std::tstring pathRemoveExtension(const std::tstring& path) {
	size_t lastsplit = path.rfind(_T('/'));
	size_t namebegin = (lastsplit == std::string::npos)
		? 0
		: lastsplit + 1;
	size_t dotpos = path.find(_T('.'), namebegin);
	size_t len = (dotpos == std::tstring::npos)
		? path.size()
		: dotpos;
	return path.substr(0, len);
}

static ENUM_ENCODER encoderFtomString(const std::tstring& str) {
	if (str == _T("x264")) {
		return ENCODER_X264;
	}
	else if (str == _T("x265")) {
		return ENCODER_X265;
	}
	else if (str == _T("qsv") || str == _T("QSVEnc")) {
		return ENCODER_QSVENC;
	}
	return (ENUM_ENCODER)-1;
}

static TranscoderSetting parseArgs(int argc, tchar* argv[])
{
	std::tstring tsFilePath;
	std::tstring outVideoPath;
	std::tstring workDir = _T("./");
	std::tstring outInfoJsonPath;
	ENUM_ENCODER encoder = ENUM_ENCODER();
	std::tstring encoderPath = _T("x264.exe");
	std::tstring encoderOptions = _T("--crf 23");
	std::tstring muxerPath = _T("muxer.exe");
	std::tstring timelineditorPath = _T("timelineeditor.exe");
	int serviceId = -1;
	bool dumpStreamInfo = bool();

	for (int i = 1; i < argc; ++i) {
		std::tstring key = argv[i];
		if (key == _T("-i") || key == _T("--input")) {
			tsFilePath = pathNormalize(getParam(argc, argv, i++));
		}
		else if (key == _T("-o") || key == _T("--output")) {
			outVideoPath =
				pathRemoveExtension(pathNormalize(getParam(argc, argv, i++)));
		}
		else if (key == _T("-w") || key == _T("--work")) {
			workDir = pathNormalize(getParam(argc, argv, i++));
		}
		else if (key == _T("-et") || key == _T("--encoder-type")) {
			std::tstring arg = getParam(argc, argv, i++);
			encoder = encoderFtomString(arg);
			if (encoder == (ENUM_ENCODER)-1) {
				printf("--encoder-type�̎w�肪�Ԉ���Ă��܂�: %" PRITSTR "\n", arg.c_str());
			}
		}
		else if (key == _T("-e") || key == _T("--encoder")) {
			encoderPath = getParam(argc, argv, i++);
		}
		else if (key == _T("-eo") || key == _T("--encoder-option")) {
			encoderOptions = getParam(argc, argv, i++);
		}
		else if (key == _T("-m") || key == _T("--muxer")) {
			muxerPath = getParam(argc, argv, i++);
		}
		else if (key == _T("-t") || key == _T("--timelineeditor")) {
			timelineditorPath = getParam(argc, argv, i++);
		}
		else if (key == _T("-j") || key == _T("--json")) {
			outInfoJsonPath = getParam(argc, argv, i++);
		}
		else if (key == _T("-s") || key == _T("--serivceid")) {
			std::tstring sidstr = getParam(argc, argv, i++);
			if (sidstr.size() > 2 && sidstr.substr(0, 2) == _T("0x")) {
				// 16�i
				serviceId = std::stoi(sidstr.substr(2), NULL, 16);;
			}
			else {
				// 10�i
				serviceId = std::stoi(sidstr);
			}
		}
		else if (key == _T("--dump")) {
			dumpStreamInfo = true;
		}
		else if (key.size() == 0) {
			continue;
		}
		else {
			THROWF(FormatException, "�s���ȃI�v�V����: %" PRITSTR, argv[i]);
		}
	}

	if (tsFilePath.size() == 0) {
		THROWF(ArgumentException, "���̓t�@�C�����w�肵�Ă�������");
	}
	if (outVideoPath.size() == 0) {
		THROWF(ArgumentException, "�o�̓t�@�C�����w�肵�Ă�������");
	}

	TranscoderSetting setting = TranscoderSetting();
	setting.tsFilePath = to_string(tsFilePath);
	setting.outVideoPath = to_string(outVideoPath);
	setting.intFileBasePath = to_string(workDir) + "/amt" + std::to_string(time(NULL));
	setting.audioFilePath = setting.intFileBasePath + "-audio.dat";
	setting.outInfoJsonPath = to_string(outInfoJsonPath);
	setting.encoder = encoder;
	setting.encoderPath = to_string(encoderPath);
	setting.encoderOptions = to_string(encoderOptions);
	setting.muxerPath = to_string(muxerPath);
	setting.timelineditorPath = to_string(timelineditorPath);
	setting.serviceId = serviceId;
	setting.dumpStreamInfo = dumpStreamInfo;

	return setting;
}

static int amatsukazeTranscodeMain(const TranscoderSetting& setting) {
	try {
		AMTContext ctx;
		transcodeMain(ctx, setting);
		return 0;
	}
	catch (Exception e) {
		return 1;
	}
}
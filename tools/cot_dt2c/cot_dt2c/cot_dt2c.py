#
# Copyright (c) 2024, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

from sys import exit

import re
import sys
import math
import os
from pathlib import Path

class bcolors:
    HEADER = '\033[95m'
    OKBLUE = '\033[94m'
    OKCYAN = '\033[96m'
    OKGREEN = '\033[92m'
    WARNING = '\033[93m'
    FAIL = '\033[91m'
    ENDC = '\033[0m'
    BOLD = '\033[1m'
    UNDERLINE = '\033[4m'

def removeNumber(s):
    result = ''.join([i for i in s if not i.isdigit()])
    return result

def extractNumber(s):
    for i in s:
        if i.isdigit():
            return (int)(i)

    return -1

def peek_line(f):
    pos = f.tell()
    line = f.readline()
    f.seek(pos)
    return line

class authMethod:
    def __init__(self):
        self.type = ""
        self.param = ""
        self.paramKey = []
        self.paramValue = []

    def printInfo(self):
        print("---------- method ------------")
        print("type:", self.type)
        print("param name:", self.param)
        print("param:")
        for i in range(len(self.paramKey)):
            print(self.paramKey[i] + ":", self.paramValue[i])

    def init_sign(self, keyname):
        if keyname == "":
            keyname = "subject_pk"
        self.type = "AUTH_METHOD_SIG"
        self.param = "sig"
        self.paramKey = ["pk", "sig", "alg", "data"]
        self.paramValue = [keyname, "sig", "sig_alg", "raw_data"]

    def init_hash(self, hash):
        self.type = "AUTH_METHOD_HASH"
        self.param = "hash"
        self.paramKey = ["data", "hash"]
        self.paramValue = ["raw_data", hash]

    def init_nv(self, name):
        self.type = "AUTH_METHOD_NV_CTR"
        self.param = "nv_ctr"
        self.paramKey = ["cert_nv_ctr", "plat_nv_ctr"]
        name = name.replace("counter", "ctr")
        self.paramValue = [name, name]

class authData:
    def __init__(self, type_desc):
        self.type_desc = type_desc
        if "sp_pkg" in type_desc:
            type_desc = removeNumber(type_desc)
        self.ptr = type_desc + "_buf"
        if "hash" in type_desc:
            self.len = "(unsigned int)HASH_DER_LEN"
        elif "pk" in type_desc:
            self.len = "(unsigned int)PK_DER_LEN"
        self.oid = ""

    def printInfo(self):
        print("--------------- data ----------------")
        print("type_desc:", self.type_desc)
        print("oid:", self.oid)
        print("ptr:", self.ptr)
        print("len:", self.len)

class image:
    def __init__(self, imageName):
        self.img_name = imageName
        self.img_id = ""
        self.parent = ""
        self.hash = ""
        self.img_type = "IMG_RAW"
        self.ifdef = []
        self.img_auth_methods = []

    def printInfo(self):
        print("============== image ==============")
        print("image name:", self.img_name)
        print("image id:", self.img_id)
        print("parent:", self.parent)
        print("hash:", self.hash)

    def validate(self):
        valid = True
        if self.img_name == "":
            print(bcolors.FAIL + "ERROR: \timage name not defined" + bcolors.ENDC)
            valid = False

        if self.img_id == "":
            print(bcolors.FAIL + "ERROR: \t{} img_id not defined".format(self.img_name) + bcolors.ENDC)
            valid = False

        if self.parent == "":
            print(bcolors.FAIL + "ERROR: \t{} refer to invalid parent".format(self.img_name) + bcolors.ENDC)
            valid = False

        return valid

class cert:
    def __init__(self, certName):
        self.cert_name = certName
        self.img_id = ""
        self.img_type = "IMG_CERT"
        self.parent = ""
        self.ifdef = []
        self.signing_key = ""
        self.antirollback_counter = ""
        self.img_auth_methods = []
        self.authenticated_data = []
        self.child = []

    def printInfo(self):
        print("===================== cert ======================")
        print("cert:", self.cert_name)
        print("image id:", self.img_id)
        print("image type:", self.img_type)
        print("parent:", self.parent)
        print("antirollback:", self.antirollback_counter)
        print("ifdef")
        print(self.ifdef)
        print("authenticated data:")
        for d in self.authenticated_data:
            d.printInfo()
        for m in self.img_auth_methods:
            m.printInfo()
        print("child:")
        print(self.child)

    def validate(self):
        valid = True
        if self.cert_name == "":
            print(bcolors.FAIL + "ERROR: \tcert name not defined" + bcolors.ENDC)
            valid = False

        if self.img_id == "":
            print(bcolors.FAIL + "ERROR: \t{} img_id not defined".format(self.cert_name) + bcolors.ENDC)
            valid = False

        for i in self.authenticated_data:
            if i.oid == "":
                print(bcolors.FAIL + "ERROR: \t{} oid is not defined".format(i.type_desc) + bcolors.ENDC)
                valid = False

        for i in self.img_auth_methods:
            for j in i.paramValue:
                if j == "":
                    print(bcolors.FAIL + "{} img auth method {} not valid".format(self.cert_name, i.type) + bcolors.ENDC)
                    valid = False

        return valid

class generic:
    def __init__(self, name):
        self.name = name
        self.id = ""
        self.oid = ""

def parseBraces(line, braces):
    if len(braces) == 1 and braces[0] == "{" and "{" in line:
        return False

    if "{" in line:
        braces.append("{")
    elif "}" in line:
        if braces[-1] != "{":
            print("invalid brackets")
            exit(1)
        else:
            braces.pop()
            if (len(braces) == 0):
                return True

    return False

def extractData(filename, dataName):
    stack = ["{"]
    reg = re.compile(r' *oid *= *([\w]+) *;')

    thisAuthData = authData(dataName)

    for line in filename:
        match = reg.search(line)
        if match != None:
            thisAuthData.oid = match.groups()[0]

        if parseBraces(line, stack):
            return thisAuthData


def extractCert(filename, certName, ifdefTag):
    stack = ["{"]
    thisCert = cert(certName)

    for i in ifdefTag:
        thisCert.ifdef.append(i)

    parent = re.compile(r'parent *= *<&([\w]+)> *;')
    imgidregex = re.compile(r'image-id *= *<([\w]+)> *;')
    keyregex = re.compile(r'signing-key *= *<&([\w]+)> *;')
    antirollbackregex = re.compile(r'antirollback-counter *= *<&([\w]+)> *;')
    dataregex = re.compile(r'([\w]+) *: *([\w]+)')

    for line in filename:

        if "root-certificate" in line:
            thisCert.parent = "NULL"
            continue

        match = parent.search(line)
        if match != None:
            thisCert.parent = match.groups()[0]
            continue

        match = imgidregex.search(line)
        if match != None:
            thisCert.img_id = match.groups()[0]
            continue

        match = keyregex.search(line)
        if match != None:
            thisCert.signing_key = match.groups()[0]
            continue

        match = antirollbackregex.search(line)
        if match != None:
            thisCert.antirollback_counter = match.groups()[0]
            continue

        match = dataregex.search(line)
        if match != None:
            word1, word2 = match.groups()
            thisData = extractData(filename, word2)
            if thisCert.parent != "NULL" and "key_cert" in thisCert.cert_name and "content_pk_buf" in thisData.ptr:
                thisData.ptr = "content_pk_buf"
            thisCert.authenticated_data.append(thisData)
            continue


        if parseBraces(line, stack):
            sign = authMethod()
            sign.init_sign(thisCert.signing_key)
            thisCert.img_auth_methods.append(sign)

            if thisCert.antirollback_counter != "":
                nv = authMethod()
                nv.init_nv(thisCert.antirollback_counter)
                thisCert.img_auth_methods.append(nv)

            return thisCert

def manifest(filename, braces, ifdefTag):
    certs = []
    cert_set = {}

    reg = re.compile(r'([\w]+) *: *([\w]+)')
    ifdefregex = re.compile(r'#if defined *\(([\w]+)\)')
    ifdefend = "#endif"

    for line in filename:
        match = reg.search(line)

        if match != None:
            word1, word2 = match.groups()
            cert_set[word2] = len(certs)
            thisCert = extractCert(filename, word2, ifdefTag)

            if thisCert.parent != "NULL":
                parent = thisCert.parent
                try:
                    certs[cert_set[parent]].child.append(thisCert.cert_name)
                except KeyError:
                    pass

            certs.append(thisCert)

        else:
            match = ifdefregex.search(line)
            if match != None:
                ifdefTag.append(match.groups()[0])

            if ifdefend in line:
                ifdefTag.pop()

            if parseBraces(line, braces):
                return certs, cert_set

def extractImage(filename, imageName, ifdefTag):
    stack = ["{"]
    thisImage = image(imageName)
    for i in ifdefTag:
        thisImage.ifdef.append(i)

    parent = re.compile(r'parent *= *<&([\w]+)> *;')
    imgidregex = re.compile(r'image-id *= *<([\w]+)> *;')
    hashregex = re.compile(r'hash *= *<&([\w]+)> *;')

    for line in filename:
        match = parent.search(line)
        if match != None:
            thisImage.parent = match.groups()[0]
            continue

        match = imgidregex.search(line)
        if match != None:
            thisImage.img_id = match.groups()[0]
            continue

        match = hashregex.search(line)
        if match != None:
            thisImage.hash = match.groups()[0]
            continue

        if parseBraces(line, stack):
            m = authMethod()
            m.init_hash(thisImage.hash)
            thisImage.img_auth_methods.append(m)
            return thisImage

def images(filename, braces, ifdefTag):
    allImages = []

    reg = re.compile(r'([\w]+) *{')
    imgNamereg = re.compile(r'([a-zA-Z0-9_]+)')
    ifdefregex = re.compile(r'#if defined *\(([\w]+)\)')
    ifdefend = "#endif"

    peekNextLine = False
    imageName = ""

    for line in filename:
        match = reg.search(line)
        match1 = imgNamereg.search(line)

        if match != None:
            imageName = match.groups()[0]
            allImages.append(extractImage(filename, imageName, ifdefTag))

        elif match1 != None:
            imageName = match1.groups()[0]
            peekNextLine = True

        elif peekNextLine:
            peekNextLine = False
            if "{" in line:
                allImages.append(extractImage(filename, imageName, ifdefTag))

        match = ifdefregex.search(line)
        if match != None:
            ifdefTag.append(match.groups()[0])

        if ifdefend in line:
            ifdefTag.pop()

        if parseBraces(line, braces):
            return allImages

def extractOther(filename, name, ifdefTag):
    stack = ["{"]

    oid = re.compile(r'oid *= *([\w]+) *;')
    id = re.compile(r'id *= *<([\w]+)> *;')

    thisGeneric = generic(name)

    for line in filename:
        match = oid.search(line)
        if match != None:
            thisGeneric.oid = match.groups()[0]
            continue

        match = id.search(line)
        if match != None:
            thisGeneric.id = match.groups()[0]
            continue

        if parseBraces(line, stack):
            return thisGeneric

def Ctrs(filename, ifdefTag):
    braces = ["{"]
    ctrs = []

    reg = re.compile(r'([\w]+) *: *([\w]+)')

    for line in filename:
        match = reg.search(line)

        if match != None:
            word1, word2 = match.groups()
            ctrs.append(extractOther(filename, word2, ifdefTag))
        else:
            if parseBraces(line, braces):
                return ctrs


def PKs(filename, ifdefTag):
    braces = ["{"]
    pks = []

    reg = re.compile(r'([\w]+) *: *([\w]+)')

    for line in filename:
        match = reg.search(line)
        if match != None:
            word1, word2 = match.groups()
            pks.append(extractOther(filename, word2, ifdefTag))

        else:
            if parseBraces(line, braces):
                return pks


def generateCert(c, f):
    for i in c.ifdef:
        f.write("#if defined({})\n".format(i))

    f.write("static const auth_img_desc_t {} = {{\n".format(c.cert_name))
    f.write("\t.img_id = {},\n".format(c.img_id))
    f.write("\t.img_type = {},\n".format(c.img_type))

    if c.parent != "NULL":
        f.write("\t.parent = &{},\n".format(c.parent))
    else:
        f.write("\t.parent = {},\n".format(c.parent))

    if len(c.img_auth_methods) != 0:
        f.write("\t.img_auth_methods = (const auth_method_desc_t[AUTH_METHOD_NUM]) {\n")
        for i, m in enumerate(c.img_auth_methods):
            f.write("\t\t[{}] = {{\n".format(i))

            f.write("\t\t\t.type = {},\n".format(m.type))
            f.write("\t\t\t.param.{} = {{\n".format(m.param))

            for j in range(len(m.paramKey)):
                f.write("\t\t\t\t.{} = &{}{}\n".format(m.paramKey[j], m.paramValue[j], "," if j != len(m.paramKey) - 1 else ""))

            f.write("\t\t\t}\n")
            f.write("\t\t}}{}\n".format("," if i != len(c.img_auth_methods) - 1 else ""))

        f.write("\t}}{}\n".format("," if len(c.authenticated_data) != 0 else ""))

    if len(c.authenticated_data) != 0:
        f.write("\t.authenticated_data = (const auth_param_desc_t[COT_MAX_VERIFIED_PARAMS]) {\n")

        for i, d in enumerate(c.authenticated_data):
            f.write("\t\t[{}] = {{\n".format(i))
            f.write("\t\t\t.type_desc = &{},\n".format(d.type_desc))
            f.write("\t\t\t.data = {\n")

            n = extractNumber(d.type_desc)
            if "pkg" not in d.type_desc or n == -1:
                f.write("\t\t\t\t.ptr = (void *){},\n".format(d.ptr))
            else:
                f.write("\t\t\t\t.ptr = (void *){}[{}],\n".format(d.ptr, n-1))

            f.write("\t\t\t\t.len = {}\n".format(d.len))
            f.write("\t\t\t}\n")

            f.write("\t\t}}{}\n".format("," if i != len(c.authenticated_data) - 1 else ""))

        f.write("\t}\n")

    f.write("};\n\n")

    for i in range(len(c.ifdef)-1, -1, -1):
        f.write("#endif /* {} */\n".format(c.ifdef[i]))

    f.write("\n")

def rawImgToCert(i, certs, cert_set):
    newCert = cert(i.img_name)
    newCert.img_id = i.img_id
    newCert.img_type = i.img_type
    newCert.parent = i.parent
    newCert.img_auth_methods = i.img_auth_methods
    newCert.ifdef = [j for j in i.ifdef]

    parent = newCert.parent
    try:
        idx = cert_set[parent]
        certs[idx].child.append(newCert.cert_name)
    except KeyError:
        pass

    certs.append(newCert)
    return newCert

def generateBuf(certs, f):
    buffers = {}
    for c in certs:
        for d in c.authenticated_data:
            buffers[d.ptr] = c.ifdef

    for key, value in buffers.items():
        for i in value:
            f.write("#if defined({})\n".format(i))

        if "sp_pkg_hash_buf" in key:
            f.write("static unsigned char {}[MAX_SP_IDS][HASH_DER_LEN];\n".format(key))
        elif "pk" in key:
            f.write("static unsigned char {}[PK_DER_LEN];\n".format(key))
        else:
            f.write("static unsigned char {}[HASH_DER_LEN];\n".format(key))

        for i in range(len(value)-1, -1, -1):
            f.write("#endif /* {} */\n".format(value[i]))

    f.write("\n")

def generateInclude(f, includeFile):
    f.write("#include <stddef.h>\n")
    f.write("#include <mbedtls/version.h>\n")
    f.write("#include <common/tbbr/cot_def.h>\n")
    f.write("#include <drivers/auth/auth_mod.h>\n")
    f.write("\n")
    for i in includeFile:
        f.write(i)
    f.write("\n")
    f.write("#include <platform_def.h>\n\n")

def generateLiscence(f, licence):
    if len(licence) != 0:
        for i in licence:
            f.write(i)

    f.write("\n")

def generateParam(certs, ctrs, pks, f):

    f.write("static auth_param_type_desc_t subject_pk = AUTH_PARAM_TYPE_DESC(AUTH_PARAM_PUB_KEY, 0);\n")
    f.write("static auth_param_type_desc_t sig = AUTH_PARAM_TYPE_DESC(AUTH_PARAM_SIG, 0);\n")
    f.write("static auth_param_type_desc_t sig_alg = AUTH_PARAM_TYPE_DESC(AUTH_PARAM_SIG_ALG, 0);\n")
    f.write("static auth_param_type_desc_t raw_data = AUTH_PARAM_TYPE_DESC(AUTH_PARAM_RAW_DATA, 0);\n")
    f.write("\n")

    for c in certs:
        if len(c.authenticated_data) != 0:
            for i in c.ifdef:
                f.write("#if defined({})\n".format(i))

        for d in c.authenticated_data:
            if "pk" in d.type_desc and "pkg" not in d.type_desc:
                f.write("static auth_param_type_desc_t {} = "\
                        "AUTH_PARAM_TYPE_DESC(AUTH_PARAM_PUB_KEY, {});\n".format(d.type_desc, d.oid))
            elif "hash" in d.type_desc:
                f.write("static auth_param_type_desc_t {} = "\
                        "AUTH_PARAM_TYPE_DESC(AUTH_PARAM_HASH, {});\n".format(d.type_desc, d.oid))
            elif "ctr" in d.type_desc:
                f.write("static auth_param_type_desc_t {} = "\
                        "AUTH_PARAM_TYPE_DESC(AUTH_PARAM_NV_CTR, {});\n".format(d.type_desc, d.oid))

        if len(c.authenticated_data) != 0:
            for i in range(len(c.ifdef)-1, -1, -1):
                f.write("#endif /* {} */\n".format(c.ifdef[i]))

    f.write("\n")

    for c in ctrs:
        f.write("static auth_param_type_desc_t {} = AUTH_PARAM_TYPE_DESC(AUTH_PARAM_NV_CTR, {});\n".format(c.name, c.oid))

    for p in pks:
        f.write("static auth_param_type_desc_t {} = AUTH_PARAM_TYPE_DESC(AUTH_PARAM_PUB_KEY, {});\n".format(p.name, p.oid))

    f.write("\n")

def generateCotDef(certs, f):
    f.write("static const auth_img_desc_t * const cot_desc[] = {\n")
    for i, c in enumerate(certs):
        for i in c.ifdef:
            f.write("#if defined({})\n".format(i))

        f.write("\t[{}]	=	&{}{}\n".format(c.img_id, c.cert_name, "," if i != len(certs) - 1 else ""))

        for i in range(len(c.ifdef)):
            f.write("#endif\n")

    f.write("};\n\n")
    f.write("REGISTER_COT(cot_desc);\n")

def generateCot(images, certs, cert_set, ctrs, pks, outputfileName, includeFile, licence):
    filename = Path(outputfileName)
    filename.parent.mkdir(exist_ok=True, parents=True)
    f = open(outputfileName, 'w+')

    generateLiscence(f, licence)
    generateInclude(f, includeFile)
    generateBuf(certs, f)

    for i in images:
        c = rawImgToCert(i, certs, cert_set)

    generateParam(certs, ctrs, pks, f)

    for c in certs:
        generateCert(c, f)

    generateCotDef(certs, f)

    f.close()
    return

def extractLicence(f):
    licence = []

    licencereg = re.compile(r'/\*')
    licenceendReg = re.compile(r'\*/')

    licencePre = False

    for line in f:
        match = licencereg.search(line)
        if match != None:
            licence.append(line)
            licencePre = True
            continue

        match = licenceendReg.search(line)
        if match != None:
            licence.append(line)
            licencePre = False
            return licence

        if licencePre:
            licence.append(line)
        else:
            return licence

    return licence

def extractInclude(f):
    include = []

    for line in f:
        if "cot" in line:
            return include

        if line != "" and "common" not in line and line != "\n":
            include.append(line)

    return include

def miniValidator(line, braces, ifdef):
    ifregex = re.compile(r'#if')
    ifdefregex = re.compile(r'#if defined *\(([\w]+)\)')
    ifdefend = "#endif"

    match = ifdefregex.search(line)
    if match != None:
        ifdef.append(match.groups()[0])

    else:
        match = ifregex.search(line)
        if match != None:
            ifdef.append("if")

    if ifdefend in line:
        if len(ifdef) == 0:
            print(bcolors.FAIL + "ERROR: \tunmatch ifdef" + bcolors.ENDC)
            print(bcolors.BOLD + "===============================================" + bcolors.ENDC)
            raise Exception("invalid DT file")
        ifdef.pop()

    if "{" in line:
        braces.append("{")
    elif "}" in line:
        if len(braces) == 0:
            print(bcolors.FAIL + "ERROR: \tunmatch brackets" + bcolors.ENDC)
            print(bcolors.BOLD + "===============================================" + bcolors.ENDC)
            raise Exception("invalid DT file")
        if braces[-1] != "{":
            print(bcolors.FAIL + "ERROR: \tunmatch brackets" + bcolors.ENDC)
            print(bcolors.BOLD + "===============================================" + bcolors.ENDC)
            raise Exception("invalid DT file")
        else:
            braces.pop()
            if (len(braces) == 0):
                return True

    return False

def validate(inputfile):
    try:
        filename = open(inputfile)
    except FileNotFoundError:
        print("input file no found")
        sys.exit(1)

    stack = []
    ifdef = []

    for line in filename:
        miniValidator(line, stack, ifdef)

    filename.close()
    if len(stack) != 0:
        print(bcolors.FAIL + "ERROR: \tunmatch brackets" + bcolors.ENDC)
        print(bcolors.BOLD + "===============================================" + bcolors.ENDC)
        raise Exception("invalid DT file")
    if len(ifdef) != 0:
        print(bcolors.FAIL + "ERROR: \tunmatch ifdef" + bcolors.ENDC)
        print(bcolors.BOLD + "===============================================" + bcolors.ENDC)
        raise Exception("invalid DT file")

def validateCert(certs, cert_set, ctr, pks, images):
    allcert = []
    valid = True
    hasRoot = False

    for c in certs:
        if c.antirollback_counter != "":
            if not any(x.name == c.antirollback_counter for x in ctr):
                print(bcolors.FAIL + "ERROR: \tnv counter {} not defined".format(c.antirollback_counter) + bcolors.ENDC)
                valid = False

        if c.parent == "NULL":
            allcert.append(c.cert_name)
            hasRoot = True

        for i in c.child:
            allcert.append(i)

        cert_valid = c.validate()
        valid = valid and cert_valid

    for i in cert_set:
        if i not in allcert:
            print(bcolors.FAIL + "ERROR: \t{} refer to invalid parent".format(i) + bcolors.ENDC)
            valid = False

    for img in images:
        img_valid = img.validate()
        valid = valid and img_valid

    if not hasRoot:
        print(bcolors.FAIL + "ERROR: \tmissing root certificate" + bcolors.ENDC)
        valid = False


    if not valid:
        print(bcolors.BOLD + "===============================================" + bcolors.ENDC)
        raise Exception("invalid DT file")

def parserMain(input):
    validate(input)

    filename = open(input)

    braces = []

    allImages = []
    certs = []
    cert_set = {}
    pks = []
    ctrs = []
    ifdefTag = []
    licence = []

    regex = re.compile(r'([\w]+) *: *([\w]+)')
    pkregex = re.compile(r'[\w]_keys')
    ifdefregex = re.compile(r'#if defined *\(([\w]+)\)')
    ifdefend = "#endif"

    licence = extractLicence(filename)
    includefile = extractInclude(filename)

    for line in filename:
        if "images" in line:
            braces.append("{")
            allImages = images(filename, braces, ifdefTag)
            continue

        if "manifests" in line:
            braces.append("{")
            certs, cert_set = manifest(filename, braces, ifdefTag)
            continue

        match = regex.search(line)
        if match != None:
            word1, word2 = match.groups()
            if "counter" in word2:
                ctrs = Ctrs(filename, ifdefTag)
            continue

        match = pkregex.search(line)
        if match != None:
            pks = PKs(filename, ifdefTag)
            continue

        match = ifdefregex.search(line)
        if match != None:
            ifdefTag.append(match.groups()[0])

        if ifdefend in line:
            ifdefTag.pop()

    validateCert(certs, cert_set, ctrs, pks, allImages)
    return allImages, certs, cert_set, ctrs, pks, includefile, licence

def generateMain(input, output):
    images, certs, cert_set, ctrs, pks, include, licence = parserMain(input)
    generateCot(images, certs, cert_set, ctrs, pks, output, include, licence)

def validateMain(input):
    parserMain(input)

if __name__=="__main__":
    if (len(sys.argv) < 2):
        print("usage: python3 " + sys.argv[0] + " [dtsi file path] [optional output c file path]")
        exit()
    if len(sys.argv) == 3:
        generateMain(sys.argv[1], sys.argv[2])
    if len(sys.argv) == 2:
        validateMain(sys.argv[1])

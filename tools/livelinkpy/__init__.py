#
#    Copyright 2023 MacKenzie Strand
#
#    Licensed under the Apache License, Version 2.0 (the "License");
#    you may not use this file except in compliance with the License.
#    You may obtain a copy of the License at
#
#       http://www.apache.org/licenses/LICENSE-2.0
#
#    Unless required by applicable law or agreed to in writing, software
#    distributed under the License is distributed on an "AS IS" BASIS,
#    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#    See the License for the specific language governing permissions and
#    limitations under the License.
#

import time
import struct
import asyncio

# ------------------ Enums ------------------ #

# ctAuditionLiveSyncCategory
CT_AUDITION_SYNC_CATEGORY_INTERNAL = 0
CT_AUDITION_SYNC_CATEGORY_CAMERA = 1
CT_AUDITION_SYNC_CATEGORY_SCENE = 2
CT_AUDITION_SYNC_CATEGORY_MATERIAL = 3

# ctAuditionLiveSyncValueType
CT_AUDITION_SYNC_NULL = 0
CT_AUDITION_SYNC_NUMBER = 1
CT_AUDITION_SYNC_BOOL = 2
CT_AUDITION_SYNC_STRING = 3
CT_AUDITION_SYNC_VEC2 = 4
CT_AUDITION_SYNC_VEC3 = 5
CT_AUDITION_SYNC_VEC4 = 6
CT_AUDITION_SYNC_QUAT = 7
CT_AUDITION_SYNC_MAT4 = 8

# ------------------ Native Types ------------------ #

class ctVec2():
    def __init__(self, x : float, y : float):
        self.x = x
        self.y = y

class ctVec3():
    def __init__(self, x : float, y : float, z : float):
        self.x = x
        self.y = y
        self.z = z

class ctVec4():
    def __init__(self, x : float, y : float, z : float, w : float):
        self.x = x
        self.y = y
        self.z = z
        self.w = w

class ctQuat():
    def __init__(self, x : float, y : float, z : float, w : float):
        self.x = x
        self.y = y
        self.z = z
        self.w = w

class ctMat4():
    def __init__(self, values : list(float)):
        self.values = values

# ------------------ Property ------------------ #

class ctAuditionLiveSyncProp:
    def __init__(self, category : int, asset : str, group : str, name : str, value : any):
        self.propCategory = category
        self.asset = asset
        self.group = group
        self.propName = name
        self.value = value
        self.timestamp = time.time()

    def GetAssetName(self) -> str:
        return self.asset

    def GetGroupName(self) -> str:
        return self.group

    def GetPropName(self) -> str:
        return self.name

    def GetTimestamp(self) -> float:
        return self.timestamp

    def GetValue(self) -> any:
        return self.value

    def Encode(self, bin : bytes):
        valueType = 0
        storage = bytes()
        if type(self.value) == None:
            valueType = CT_AUDITION_SYNC_NULL
        elif type(self.value) == float:
            valueType = CT_AUDITION_SYNC_NUMBER
            storage = struct.pack("d", self.value)
        elif type(self.value) == int:
            valueType = CT_AUDITION_SYNC_NUMBER
            storage = struct.pack("d", float(self.value))
        elif type(self.value) == bool:
            valueType = CT_AUDITION_SYNC_BOOL
            storage = struct.pack("?", self.value)
        elif type(self.value) == str:
            valueType = CT_AUDITION_SYNC_STRING
            storage = struct.pack("63sx", self.value)
        elif type(self.value) == ctVec2:
            valueType = CT_AUDITION_SYNC_VEC2
            storage = struct.pack("ff", self.value.x, self.value.y)
        elif type(self.value) == ctVec3:
            valueType = CT_AUDITION_SYNC_VEC3
            storage = struct.pack("fff", self.value.x, self.value.y, self.value.z)
        elif type(self.value) == ctVec4:
            valueType = CT_AUDITION_SYNC_VEC4
            storage = struct.pack("ffff", self.value.x, self.value.y, self.value.z, self.value.w)
        elif type(self.value) == ctQuat:
            valueType = CT_AUDITION_SYNC_QUAT
            storage = struct.pack("ffff", self.value.x, self.value.y, self.value.z, self.value.w)
        elif type(self.value) == ctMat4:
            valueType = CT_AUDITION_SYNC_MAT4
            storage = struct.pack("16f", *self.value)
        else:
            raise TypeError(f"Cannot serialize {self.propName} of type {type(self.value)}!")

        return struct.pack("BB29sx31sx31sx64s", self.propCategory, valueType, self.propName, self.asset, self.group, storage)

    def Decode(self, bin : bytes):
        valueType
        storage
        self.propCategory, valueType, self.propName, self.asset, self.group, storage = struct.unpack("BB29sx31sx31sx64s", bin)

        if valueType == CT_AUDITION_SYNC_NULL:
            self.value = None
        elif valueType == CT_AUDITION_SYNC_NUMBER:
            self.value = struct.unpack("d", storage)
        elif valueType == CT_AUDITION_SYNC_BOOL:
            self.value = struct.unpack("?", storage)
        elif valueType == CT_AUDITION_SYNC_STRING:
            bdata = struct.unpack("63sx", storage)
            self.value = bdata.decode('utf-8')
        elif valueType == CT_AUDITION_SYNC_VEC2:
            x, y = struct.unpack("ff", storage)
            self.value = ctVec2(x,y)
        elif valueType == CT_AUDITION_SYNC_VEC3:
            x, y, z = struct.unpack("fff", storage)
            self.value = ctVec3(x,y,z)
        elif valueType == CT_AUDITION_SYNC_VEC4:
            x, y, z, w = struct.unpack("ffff", storage)
            self.value = ctVec4(x,y,z,w)
        elif valueType == CT_AUDITION_SYNC_QUAT:
            x, y, z, w = struct.unpack("ffff", storage)
            self.value = ctQuat(x,y,z,w)
        elif valueType == CT_AUDITION_SYNC_MAT4:
            vals = struct.unpack("16f", storage)
            self.value = ctMat4(vals)
        else:
            raise TypeError(f"Cannot deserialize {self.propName} of unknown type number {valueType}!")

# ------------------ Client ------------------ #

class ctAuditionLiveSyncClient:
    def __init__(self):
        pass

    def Connect(self, address : str = "localhost", port : int = 4892):
        pass

    def Disconnect(self):
        pass

    def SendProp(self, prop : ctAuditionLiveSyncProp):
        pass

    def Dispatch(self, category : int, callback : function(ctAuditionLiveSyncProp, any), userData : any):
        pass

# ------------------ Test ------------------ #

def test():
    client = ctAuditionLiveSyncClient()
    client.Connect()
    time.sleep(1.0)
    client.SetProp(ctAuditionLiveSyncProp(CT_AUDITION_SYNC_CATEGORY_CAMERA, "TEST", "NOGROUP", "foo", ctVec3(0,0,0)))
    print(test)
    client.Disconnect()

if __name__ == "__main__":
    test()
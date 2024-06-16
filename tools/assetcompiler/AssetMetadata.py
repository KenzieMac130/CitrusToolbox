import io
import json
import uuid
from pathlib import Path

class AssetMetaData:
    nickname = None
    args = {}
    guids = {}
    type = "unknown"
    raw_json = {}

    def __init__(self, fp = None, dict = None) -> None:
        if fp:
            data = json.load(fp)
            self.read(data)
        if dict:
            self.read(dict)
        else:
            self.guids["OUTPUT"] = uuid.uuid4()
        
    def read(self, data):
        self.raw_json = data
        if "nickname" in data:
            self.nickname = data["nickname"]
        if "args" in data:
            self.args = data["args"]
        if "guids" in data:
            text_guids = data["guids"]
            for key, value in text_guids.items():
                self.guids[key] = uuid.UUID(value)
        if "type" in data:
            self.type = data["type"]

    def dump(self, fp):
        fp.write(self.dumps())

    def dumps(self) -> str:
        dump = self.raw_json
        if self.nickname:
            dump["nickname"] = self.nickname
        else:
            try:
                dump.pop("nickname")
            except KeyError:
                pass
        dump["args"] = self.args
        for key, value in self.guids.items():
            dump[key] = value.hex
        dump["type"] = self.type
        return json.dumps(dump, indent=2)

    def regenerate_guids(self):
        new_guids = {}
        for key in self.guids.keys():
            new_guids[key] = uuid.uuid4()
        self.guids = new_guids

    def check_where_nickname_used(nickname: str, ignore_if_guid: uuid = None, path=Path('.')) -> Path:
        for p in path.rglob("*.ctac"):
            fp = p.open()
            meta = AssetMetaData()
            meta.read(fp)
            fp.close()
            # skip if we are checking an asset against itself
            if ignore_if_guid:
                if ignore_if_guid in meta.guids.values():
                    continue
            if meta.nickname == nickname:
                return p
        return None
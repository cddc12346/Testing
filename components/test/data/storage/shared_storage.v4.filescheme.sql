PRAGMA foreign_keys=OFF;
BEGIN TRANSACTION;
CREATE TABLE meta(key LONGVARCHAR NOT NULL UNIQUE PRIMARY KEY, value LONGVARCHAR);
INSERT INTO meta VALUES('mmap_status','-1');
INSERT INTO meta VALUES('version','4');
INSERT INTO meta VALUES('last_compatible_version','4');
CREATE TABLE values_mapping(context_origin TEXT NOT NULL,key BLOB NOT NULL,value BLOB NOT NULL,last_used_time INTEGER NOT NULL,PRIMARY KEY(context_origin,key)) WITHOUT ROWID;
INSERT INTO "values_mapping" VALUES ('http://google.com',X'6b00650079003100',X'760061006c00750065003100',13312097333991364);
INSERT INTO "values_mapping" VALUES ('http://google.com',X'6b00650079003200',X'760061006c00750065003200',13313037427966159);
INSERT INTO "values_mapping" VALUES ('http://youtube.com',X'7600690073006900740065006400',X'3100310031003100310031003100',13313037435619704);
INSERT INTO "values_mapping" VALUES ('file://',X'6100',X'',13313037416916308);
INSERT INTO "values_mapping" VALUES ('file://',X'6200',X'680065006c006c006f00',13312097333991364);
INSERT INTO "values_mapping" VALUES ('file://',X'6300',X'67006f006f006400620079006500',13312353831182651);
INSERT INTO "values_mapping" VALUES ('http://google.org',X'3100',X'6600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600',13269546476192362);
INSERT INTO "values_mapping" VALUES ('http://google.org',X'3200',X'3b00',13269546476192362);
INSERT INTO "values_mapping" VALUES ('http://google.org',X'2300',X'5b005d00',13269546476192362);
INSERT INTO "values_mapping" VALUES ('http://google.org',X'6600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600660066006600',X'6b00',13269546476192362);
CREATE TABLE per_origin_mapping(context_origin TEXT NOT NULL PRIMARY KEY,creation_time INTEGER NOT NULL,length INTEGER NOT NULL) WITHOUT ROWID;
INSERT INTO "per_origin_mapping" VALUES ('http://google.com',13266954476192362,2);
INSERT INTO "per_origin_mapping" VALUES ('http://youtube.com',13266954593856733,1);
INSERT INTO "per_origin_mapping" VALUES ('file://',13268941676192362,3);
INSERT INTO "per_origin_mapping" VALUES ('http://google.org',13269546476192362,4);
CREATE TABLE budget_mapping(id INTEGER NOT NULL PRIMARY KEY,context_site TEXT NOT NULL,time_stamp INTEGER NOT NULL,bits_debit REAL NOT NULL);
INSERT INTO "budget_mapping" VALUES (1,'http://google.com',13266954476192332,1.0);
INSERT INTO "budget_mapping" VALUES (2,'http://google.com',13266954476192344,2.0);
INSERT INTO "budget_mapping" VALUES (3,'http://google.com',13266954476192362,1.1);
INSERT INTO "budget_mapping" VALUES (4,'http://youtube.com',13266954593856693,1.3);
INSERT INTO "budget_mapping" VALUES (5,'file://',13268941676192362,1.5);
INSERT INTO "budget_mapping" VALUES (6,'http://youtube.com',13266954593856733,2.3);
INSERT INTO "budget_mapping" VALUES (13,'http://google.org',13269546476192362,4);
CREATE INDEX budget_mapping_site_time_stamp_idx ON budget_mapping(context_site,time_stamp);
CREATE INDEX values_mapping_last_used_time_idx ON values_mapping(last_used_time);
CREATE INDEX per_origin_mapping_creation_time_idx ON per_origin_mapping(creation_time);
COMMIT;
-- shafs example schema
-- hardcoded in shafs.c

CREATE TABLE shafs(
    filepath VARCHAR(4096) PRIMARY KEY, -- sqlite doesnt care
    filehash CHAR(64),
    filesize INTEGER NOT NULL DEFAULT 0
);

CREATE INDEX by_filehash ON shafs(filehash);
CREATE INDEX by_filesize ON shafs(filesize);
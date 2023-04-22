package ru.ctf.scpql;

import java.io.IOException;
import java.time.OffsetDateTime;
import java.time.ZoneOffset;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Properties;
import java.util.regex.Pattern;

import ru.ctf.doc.Content;
import ru.ctf.doc.DocWithOwner;
import ru.ctf.doc.Document;
import ru.ctf.doc.dao.DAO;
import ru.ctf.doc.dao.DocumentDAO;
import ru.ctf.user.User;

public class SCPQL {
    private static final Pattern numPattern = Pattern.compile("\\d+");
    private static PragmaAuth pragmaAuth;
    private User user;
    private final Map<String, Document> docs;
    private final Map<String, Value<?>> vars;
    private final DAO<Long, Document> dao;
    private Result result;

    static {
        try {
            var prop = new Properties();
            prop.load(SCPQL.class.getClassLoader().getResourceAsStream("server.properties"));
            String pragmaAuthProp = prop.getProperty("server.pragmaAuth", "scpql");
            SCPQL.pragmaAuth = PragmaAuth.fromString(pragmaAuthProp);
        } catch (IOException e) {
            throw new RuntimeException(e);
        }
    }

    public SCPQL(User queryWriter) {
        this.user = queryWriter;
        this.dao = DocumentDAO.getInstance();
        this.vars = new HashMap<>();
        this.docs = new HashMap<>();
    }

    public void process(String code) {
        var lines = code.replaceAll("\\s", "").split(";");
        for (var line : lines) {
            if (line.startsWith("@")) {
                this.processVar(line);
            } else if (line.startsWith("doc_")) {
                this.processDocField(line);
            } else if (line.startsWith("{") && line.endsWith("}")) {
                this.processFunc(line);
            }
        }
    }

    private Value<?> processFunc(String line) {
        line = line.substring(1, line.length() - 1);
        var funcParts = line.split("->");
        if (funcParts.length != 2) {
            throw new RuntimeException();
        }
        var argsStr = funcParts[0].substring(1, funcParts[0].length() - 1).split(",");
        List<Value<?>> args = new ArrayList<>();
        for (var arg : argsStr) {
            arg = arg.strip();
            if (arg.startsWith("@")) {
                args.add(this.getVarValue(arg));
            } else {
                args.add(this.getValue(arg));
            }
        }
        var funcName = funcParts[1].strip();
        switch (funcName) {
            case ("save") -> {
                if (args.size() != 1) {
                    throw new RuntimeException();
                }
                return processSave(args);
            }
            case ("get") -> {
                if (args.size() != 1) {
                    throw new RuntimeException();
                }
                return processGet(args);
            }
            case ("secret") -> {
                if (args.size() != 2) {
                    throw new RuntimeException();
                }
                return processSecret(args);
            }
            case ("result") -> {
                if (args.size() != 1) {
                    throw new RuntimeException();
                }
                return processResult(args);
            }
            case ("list") -> {
                if (args.size() != 2) {
                    throw new RuntimeException();
                }
                return processGetList(args);
            }
        }
        return new EmptyValue();
    }

    private void processVar(String line) {
        var parts = line.split("<-");
        if (parts.length != 2) {
            throw new RuntimeException();
        }
        if (parts[1].startsWith("{") && parts[1].endsWith("}")) {
            this.vars.put(parts[0].strip(), this.processFunc(parts[1]));
        } else {
            this.vars.put(parts[0].strip(), this.getValue(parts[1].strip()));
        }
    }

    private void processDocField(String line) {
        var parts = line.split("<-");
        if (parts.length != 2) {
            throw new RuntimeException();
        }
        var docDesc = parts[0].replaceAll("doc_", "").split("_");
        if (docDesc.length != 2) {
            throw new RuntimeException();
        }
        var title = docDesc[0].strip();
        var field = docDesc[1].strip();
        var variable = parts[1].strip();
        Value<?> value;
        if (variable.startsWith("@")) {
            value = this.getVarValue(variable);
        } else {
            value = this.getValue(variable);
        }
        var doc = this.docs.get(title);
        if (doc == null) {
            var content = new Content();
            doc = new Document(title, user, content, OffsetDateTime.now(ZoneOffset.UTC));
            this.docs.put(title, doc);
        }
        doc.content().setField(field, value);
    }

    private Value<?> getValue(String value) {
        if (value.startsWith("\"") && value.endsWith("\"")) {
            return new StrValue(value.replaceAll("\"", ""));
        } else if (numPattern.matcher(value).matches()) {
            return new IntValue(value);
        } else if (value.startsWith("scp")) {
            return new DocValue(value);
        } else {
            throw new RuntimeException();
        }
    }

    private Value<?> getVarValue(String variable) {
        var value = this.vars.get(variable);
        if (value == null) {
            throw new RuntimeException();
        }
        return value;
    }

    private Value<?> processSave(List<Value<?>> args) {
        DocValue docValue = (DocValue) args.get(0);
        if (docValue.getType() != Type.DOC) {
            throw new RuntimeException();
        }
        var docTitle = docValue.getValue();
        var doc = this.docs.get(docTitle);
        if (doc == null) {
            throw new RuntimeException();
        }
        var docId = this.dao.save(doc);
        return new IntValue(Long.toString(docId));
    }

    private Value<?> processGet(List<Value<?>> args) {
        IntValue intValue = (IntValue) args.get(0);
        if (intValue.getType() != Type.INT) {
            throw new RuntimeException();
        }
        var doc = this.dao.get((long) intValue.getValue());
        if (doc == null) {
            throw new RuntimeException();
        }
        this.docs.put(doc.title(), doc);
        return new DocValue(doc.title());
    }

    private Value<?> processGetList(List<Value<?>> args) {
        if (args.get(0).type != Type.INT || args.get(1).type != Type.INT) {
            throw new RuntimeException();
        }
        var offset = (int) args.get(0).getValue();
        var limit = (int) args.get(1).getValue();
        if (limit < 0 || offset < 0) {
            throw new RuntimeException();
        }
        if (limit > 100) {
            limit = 100;
        }
        List<DocWithOwner> docs = this.dao.getAll(offset, limit);
        var valBuilder = new StringBuilder();
        for (var doc : docs) {
            valBuilder.append(doc.docId()).append(": ").append(doc.owner()).append("\n");
        }
        return new StrValue(valBuilder.toString());
    }

    private Value<?> processSecret(List<Value<?>> args) {
        if (args.get(0).getType() != Type.DOC || args.get(1).getType() != Type.STRING) {
            throw new RuntimeException();
        }
        DocValue docValue = (DocValue) args.get(0);
        StrValue fieldName = (StrValue) args.get(1);
        var doc = this.docs.get(docValue.getValue());
        if (doc == null) {
            throw new RuntimeException();
        }
        var field = doc.content().getFields().get(fieldName.getValue());
        if (field == null) {
            throw new RuntimeException();
        }
        field.setSecret(true);
        return new EmptyValue();
    }

    private void accessValue(Value<?> val) {
        if (val.getType() != Type.DOC) {
            return;
        }
        var docValue = (DocValue) val;
        if (pragmaAuth == PragmaAuth.SCPQL) {
            var userVal = vars.get(Specials.USER_SPACE.toString());
            if (userVal != null && userVal.getType() == Type.STRING) {
                var userStrVal = (StrValue) userVal;
                user = new User(userStrVal.getValue());
            }
        }
        var doc = docs.get(docValue.getValue());
        if (doc == null) {
            throw new RuntimeException();
        }
        var owner = doc.owner();
        if (!owner.login().equals(user.login())) {
            for (var field : doc.content().getFields().entrySet()) {
                if (field.getValue().isSecret()) {
                    field.setValue(new StrValue("ДАННЫЕ УДАЛЕНЫ"));
                }
            }
        }
    }

    private Value<?> processResult(List<Value<?>> args) {
        accessValue(args.get(0));
        result = args.get(0).toResult(vars, docs);
        return new EmptyValue();
    }

    public Result getResult() {
        return this.result;
    }
}

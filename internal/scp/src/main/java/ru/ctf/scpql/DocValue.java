package ru.ctf.scpql;

import java.util.Map;

import ru.ctf.TemplatesHolder;
import ru.ctf.doc.Document;

public class DocValue extends Value<String>{
    public DocValue(String val) {
        this.value = val.replaceAll("doc_", val);
        this.type = Type.DOC;
    }

    @Override
    public Result toResult(Map<String, Value<?>> vars, Map<String, Document> docs) {
        var doc = docs.get(value);
        if (doc == null) {
            throw new RuntimeException();
        }
//            BufferedReader templateReader = new BufferedReader(new FileReader(doc.templateName()));
//            var templateStringBuilder = new StringBuilder();
//            String line;
//            while ((line = templateReader.readLine()) != null) {
//                templateStringBuilder.append(line);
//                templateStringBuilder.append("\n");
//            }
//            var template = new Template(templateStringBuilder.toString());
        var template = TemplatesHolder.getTemplate(doc.templateName());
        return new Result(ResultType.HTML, template.getResultHtml(doc.content()));
    }
}

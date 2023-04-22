package ru.ctf.http;

import java.util.ArrayList;
import java.util.Collections;
import java.util.Comparator;
import java.util.List;

import org.graalvm.collections.Pair;
import ru.ctf.doc.Content;

public record Template(String payload) {

    public String getResultHtml(Content content) {
        return insertFields(content);
    }

    private String insertFields(Content content) {
        var payloadBuilder = new StringBuilder(payload);
        var toInsert = this.getInsertIndexes(payload, content.getFields().size());
        int offset = 0;
        var fields = content.getFields().entrySet().stream().toList();
        for (int i = 0; i < content.getFields().size(); i++) {
            var pair = toInsert.get(i);
            var left = pair.getLeft();
            var right = pair.getRight();
            payloadBuilder.delete(left + offset, right + offset);
            var fieldStr = String.format("<a class=\"spoiler\">%s</a>", fields.get(i).getValue().toString());
            payloadBuilder.insert(left + offset, fieldStr);
            offset += fieldStr.length() - (right - left);
        }
        return payloadBuilder.toString();
    }

    private List<Pair<Integer, Integer>> getInsertIndexes(String payload, int fieldToInsert) {
        var cPayload = payload.toCharArray();
        var possiblePlaces = new ArrayList<Pair<Integer, Integer>>();
        int startPos = 0;
        for (int i = 0; i < cPayload.length; i++) {
            if (cPayload[i] != ' ' && cPayload[i] != '\n' && cPayload[i] != '\t') {
                continue;
            }
            possiblePlaces.add(Pair.create(startPos, i));
            startPos = i+1;
        }
        Collections.shuffle(possiblePlaces);
        var places = possiblePlaces.subList(0, fieldToInsert);
        places.sort(Comparator.comparingLong(Pair::getLeft));
        return places;
    }
}

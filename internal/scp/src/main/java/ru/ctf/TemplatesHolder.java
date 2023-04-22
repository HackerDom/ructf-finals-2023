package ru.ctf;

import java.io.File;
import java.io.IOException;
import java.nio.file.Files;
import java.nio.file.Path;
import java.util.HashMap;
import java.util.Map;
import java.util.Objects;
import java.util.Random;

import ru.ctf.http.Template;

public class TemplatesHolder {
    private static final Map<String, Template> templateFiles = new HashMap<>();

    private static final String defaultFileName = "1";

    static {
        try {
            var templatesDir = new File("./templates");
            for (var file : Objects.requireNonNull(templatesDir.listFiles())) {
                templateFiles.put(file.getName(), new Template(Files.readString(Path.of(file.toURI()))));
            }
        } catch (IOException | NullPointerException ex) {
            ex.printStackTrace();
            System.exit(1);
        }
    }

    public static Template getTemplate(String name) {
        return templateFiles.get(name);
    }

    public static String getRandomName() {
        int rnd = new Random().nextInt(templateFiles.size());
        return templateFiles.keySet().stream().skip(rnd).findFirst().orElse(defaultFileName);
    }
}

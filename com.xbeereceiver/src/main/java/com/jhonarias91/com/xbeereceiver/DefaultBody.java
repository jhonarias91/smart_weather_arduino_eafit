package com.jhonarias91.com.xbeereceiver;

public class DefaultBody {
    private String content; // O el nombre que tengas en el JSON

    // Getters y setters
    public String getContent() {
        return content;
    }

    public void setContent(String content) {
        this.content = content;
    }

    @Override
    public String toString() {
        return "{" +
                "content='" + content + '\'' +
                '}';
    }
}


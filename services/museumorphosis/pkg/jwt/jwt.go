package jwt

import (
	"fmt"
	"github.com/golang-jwt/jwt/v5"
)

func GenToken(id string, secret string) (string, error) {
	token := jwt.New(jwt.SigningMethodHS256)
	claims := token.Claims.(jwt.MapClaims)

	claims["id"] = id

	tokenString, err := token.SignedString([]byte(secret))
	if err != nil {
		return "", fmt.Errorf("error while generate token: %w", err)
	}
	return tokenString, nil
}

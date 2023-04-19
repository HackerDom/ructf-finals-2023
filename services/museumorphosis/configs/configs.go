package configs

import (
	"github.com/google/uuid"
	"github.com/spf13/viper"
	"os"
)

type Configs struct {
	Port      int `mapstructure:"APP_PORT"`
	JWTSecret string
}

var Cfg *Configs

func Init() (*Configs, error) {
	viper.SetConfigFile(".env")
	if err := viper.ReadInConfig(); err != nil {
		return nil, err
	}
	Cfg = &Configs{}
	if err := viper.Unmarshal(Cfg); err != nil {
		return nil, err
	}
	secret, err := generateSecretIfNotExits()
	if err != nil {
		return nil, err
	}
	Cfg.JWTSecret = secret
	return Cfg, nil
}

func generateSecretIfNotExits() (string, error) {
	b, err := os.ReadFile("jwt_secret/secret")
	if err != nil {
		return "", err
	}
	secret := string(b)

	if len(secret) == 0 {
		secretUid, err := uuid.NewRandom()
		if err != nil {
			return "", err
		}
		secret = secretUid.String()
		if err := os.WriteFile("jwt_secret/secret", []byte(secret), 0666); err != nil {
			return "", err
		}
	}
	return secret, nil
}

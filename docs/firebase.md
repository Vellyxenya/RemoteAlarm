# Firebase Configuration Files

## .firebaserc
Copy `.firebaserc.template` to `.firebaserc` and update with your Firebase project ID:

```bash
cp .firebaserc.template .firebaserc
```

Then edit `.firebaserc`:
```json
{
  "projects": {
    "default": "your-actual-project-id"
  }
}
```

## What's Configured

- **Storage Rules** (`storage.rules`): Security rules for Firebase Storage
- **Functions** (`firebase.json`): Configuration for Cloud Functions deployment
- **Project Link** (`.firebaserc`): Links local project to Firebase project

## Deployment

Deploy storage rules:
```bash
firebase deploy --only storage
```

Deploy everything:
```bash
firebase deploy
```

## See Also

- [Complete Setup Guide](docs/firebase-setup.md)
- [Security Documentation](docs/security.md)

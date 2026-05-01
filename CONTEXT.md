# VitaDeck

VitaDeck lets people run custom interactive deck-style interfaces on a PlayStation Vita.

## Language

**Deck App**:
A user-provided interactive VitaDeck experience rendered by the VitaDeck runtime.
_Avoid_: Plugin, applet, experience, deck

**Deck App Source Entry**:
The source-level React entry file that is compiled into a **Deck App** bundle.
_Avoid_: Plugin file, component file

**Deck App Component**:
The default-exported React component from a **Deck App Source Entry**.
_Avoid_: Render function, bootstrap

**Deck App Manifest**:
The metadata file that identifies a **Deck App** and its **Deck App Source Entry**.
_Avoid_: Package file, plugin config

**Active Deck App**:
The **Deck App** currently selected to be bundled into VitaDeck.
_Avoid_: Current plugin, selected component

**Deck App Workspace**:
The in-repository area where MVP **Deck Apps** are authored.
_Avoid_: Plugin folder, examples folder

**Runtime Upload**:
A future installation flow where a **Deck App** is sent to a running VitaDeck instance over the network.
_Avoid_: Hot reload, live edit

**Render Surface**:
The full Vita display area owned by the currently running **Deck App**.
_Avoid_: Page, route, viewport

**VitaDeck Runtime API**:
The public author-facing API used by **Deck Apps** to render UI and access VitaDeck capabilities.
_Avoid_: Native bindings, intrinsic elements, host config

**Runtime Theme**:
The VitaDeck-provided styling palette available to **Deck Apps** through the **VitaDeck Runtime API**.
_Avoid_: Sample theme, app-local theme

**Screen**:
The root **VitaDeck Runtime API** component representing the full Vita display.
_Avoid_: Surface, viewport, canvas

**Button**:
The MVP **VitaDeck Runtime API** component for user interaction.
_Avoid_: Pressable rect, clickable shape

**Press**:
The VitaDeck input gesture used by public **Button** callbacks.
_Avoid_: Click, mouse down, mouse up, hover

## Relationships

- A **Deck App** is rendered by the VitaDeck runtime.
- A **Deck App** is authored against the **VitaDeck Runtime API**.
- A **Deck App** owns the full **Render Surface** in the MVP.
- A **Deck App Source Entry** is bundled at build time for the MVP.
- A **Deck App Source Entry** default-exports one **Deck App Component**.
- A **Deck App Manifest** contains `name` and `entry` for the MVP.
- A **Deck App Manifest** selects which **Deck App Source Entry** is bundled.
- The project configuration selects the **Active Deck App** by pointing at a **Deck App Manifest**.
- The **Deck App Workspace** contains MVP Deck Apps under `deck-apps/`.
- VitaDeck owns bootstrapping and renders the **Deck App Component**.
- The **VitaDeck Runtime API** may be implemented as a thin local TypeScript module for the MVP.
- The **VitaDeck Runtime API** provides the **Runtime Theme**.
- **Deck Apps** consume the **Runtime Theme**; VitaDeck owns the theme provider.
- The **Screen** component is the root of a **Deck App** UI.
- A **Deck App Component** explicitly renders **Screen**.
- **Button** is the only public interactable UI component in the MVP.
- **Button** exposes press-oriented callbacks: `onPress`, `onPressStart`, and `onPressEnd`.
- **Runtime Upload** should eventually use an HTTP server running on the Vita.

## Example dialogue

> **Dev:** "Can this React component be the whole thing VitaDeck runs?"
> **Domain expert:** "For MVP language, call the runnable thing a **Deck App**; a React component may be its entry point, but the Deck App is the user-facing unit."
>
> **Dev:** "Should MVP support uploading Deck Apps while VitaDeck is already running?"
> **Domain expert:** "No — MVP can bundle a **Deck App Source Entry** at build time, but the model should leave room for **Runtime Upload** later."
>
> **Dev:** "Does VitaDeck keep its own navigation UI around the Deck App?"
> **Domain expert:** "No — for MVP, the running **Deck App** owns the full **Render Surface**."
>
> **Dev:** "Should Deck App authors use raw `<vita-button>` tags?"
> **Domain expert:** "No — expose a **VitaDeck Runtime API** so the author contract can stay stable while internals evolve."
>
> **Dev:** "Should each sample app carry its own theme helper?"
> **Domain expert:** "No — theme is a **Runtime Theme** provided by the **VitaDeck Runtime API**."
>
> **Dev:** "Should Deck Apps mount the theme provider?"
> **Domain expert:** "No — VitaDeck provides it during bootstrap; Deck Apps only consume the **Runtime Theme**."
>
> **Dev:** "What should the root UI component be called?"
> **Domain expert:** "Use **Screen**; it represents the Vita display that the Deck App owns."
>
> **Dev:** "Should VitaDeck wrap Deck Apps in Screen automatically?"
> **Domain expert:** "No — a **Deck App Component** explicitly renders **Screen** so display ownership is visible in author code."
>
> **Dev:** "Can any visual element receive clicks?"
> **Domain expert:** "No — **Button** is the only public interactable component in the MVP."
>
> **Dev:** "Should Deck Apps use mouse event names?"
> **Domain expert:** "No — public input is described as **Press**, using `onPress`, `onPressStart`, and `onPressEnd`."
>
> **Dev:** "Should a Deck App call render itself?"
> **Domain expert:** "No — the **Deck App Source Entry** default-exports a **Deck App Component**, and VitaDeck owns rendering it."
>
> **Dev:** "Does the Vita runtime need to read the manifest in the MVP?"
> **Domain expert:** "No — the **Deck App Manifest** is build-time metadata for now, but it should be suitable to travel with a future Runtime Upload package."
>
> **Dev:** "How do I avoid passing the Deck App path every time I build?"
> **Domain expert:** "Use project configuration to choose the **Active Deck App** once, then normal build commands package it."

## Flagged ambiguities

- "React component" can mean either a source-level entry component or the whole runnable **Deck App**; resolved: **Deck App** is the runnable unit.
- "upload" can mean build-time packaging or **Runtime Upload**; resolved: MVP uses build-time bundling, while **Runtime Upload** is a future HTTP-based flow.
- "UI elements" can mean public author-facing components or internal host elements; resolved: **Deck Apps** use the **VitaDeck Runtime API**, while host elements remain internal.
- "theme" can mean sample app styling or runtime styling; resolved: the shared theme is a **Runtime Theme** owned by VitaDeck.
- "clickable rect" suggests visual primitives are interactive; resolved: public interaction goes through **Button**.
- "click" and "mouse" terms are implementation leakage; resolved: public input language is **Press**.
- "Surface" and "viewport" were considered for the root display component; resolved: the public component is **Screen**.
- "render function" suggests author-controlled bootstrapping; resolved: authors export a **Deck App Component** and VitaDeck bootstraps it.
- "manifest" could mean runtime configuration or build metadata; resolved: **Deck App Manifest** is build-time metadata in the MVP.
- "selected app" is the **Active Deck App**; resolved: the project configuration points to its **Deck App Manifest**.
